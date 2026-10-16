#include <iostream>
#include <string>
#include <map>
#include "http_context.h"
#include "http_request.h"
#include "buffer.h"

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond)                                                     \
    do {                                                                \
        if (cond) {                                                     \
            ++g_pass;                                                   \
        } else {                                                        \
            ++g_fail;                                                   \
            std::cerr << "  [FAIL] " << __LINE__ << ": " << #cond       \
                      << std::endl;                                     \
        }                                                               \
    } while (0)

// 打印一个请求的全部解析结果，便于人工观察
static void DumpRequest(const HttpRequest &request) {
    std::cout << "  method : " << request.method_str() << std::endl;
    std::cout << "  url    : " << request.url() << std::endl;
    std::cout << "  version: " << request.version_str() << std::endl;

    std::cout << "  params : ";
    for (const auto &it : request.request_params())
        std::cout << "[" << it.first << "=" << it.second << "] ";
    std::cout << std::endl;

    std::cout << "  headers: " << request.header().size() << " 个" << std::endl;
    for (const auto &it : request.header())
        std::cout << "    " << it.first << " = " << it.second << std::endl;

    std::cout << "  body   : \"" << request.body() << "\"" << std::endl;
}

// ---------------------------------------------------------------
// 用例1: 完整 GET 请求（真实浏览器报文）
// ---------------------------------------------------------------
static void TestGetRequest() {
    std::cout << "=== TestGetRequest ===" << std::endl;

    std::string str = "GET /hello?a=2 HTTP/1.1\r\n"
                      "Host: 127.0.0.1:1234\r\n"
                      "Connection: keep-alive\r\n"
                      "Cache-Control: max-age=0\r\n"
                      "sec-ch-ua: \"Google Chrome\";v=\"113\", \"Chromium\";v=\"113\", \"Not-A.Brand\";v=\"24\"\r\n"
                      "sec-ch-ua-mobile: ?0\r\n"
                      "sec-ch-ua-platform: \"Linux\"\r\n"
                      "Upgrade-Insecure-Requests: 1\r\n"
                      "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/113.0.0.0 Safari/537.36\r\n"
                      "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7\r\n"
                      "Sec-Fetch-Site: none\r\n"
                      "Sec-Fetch-Mode: navigate\r\n"
                      "Sec-Fetch-User: ?1\r\n"
                      "Sec-Fetch-Dest: document\r\n"
                      "Accept-Encoding: gzip, deflate, br\r\n"
                      "Accept-Language: zh-CN,zh;q=0.9,en;q=0.8,zh-TW;q=0.7\r\n"
                      "Cookie: username-127-0-0-1-8888=\"2|1:0|10:1681994652|23:username-127-0-0-1-8888|44:Yzg5ZjA1OGU0MWQ1NGNlMWI2MGQwYTFhMDAxYzY3YzU=|6d0b051e144fa862c61464acf2d14418d9ba26107549656a86d92e079ff033ea\"; _xsrf=2|dd035ca7|e419a1d40c38998f604fb6748dc79a10|168199465\r\n"
                      "\r\n";

    HttpContext context;
    Buffer buf;
    buf.set_buf(str);

    CHECK(context.ParseRequest(&buf) == HttpContext::ParseResult::kGotAll);
    CHECK(context.GotAll());

    const HttpRequest &request = context.request();
    DumpRequest(request);

    CHECK(request.method() == HttpRequest::Method::kGet);
    CHECK(request.version() == HttpRequest::Version::kHttp11);
    CHECK(request.url() == "/hello");
    CHECK(request.request_params().size() == 1);
    CHECK(request.get_request_value("a") == "2");

    // 首部字段名统一小写存储，查询时大小写不敏感
    CHECK(request.header().size() == 16);
    CHECK(request.get_header_value("Host") == "127.0.0.1:1234");
    CHECK(request.get_header_value("host") == "127.0.0.1:1234");
    CHECK(request.get_header_value("CONTENT-LENGTH").empty());
    CHECK(request.get_header_value("sec-ch-ua-mobile") == "?0");
    CHECK(request.body().empty());

    // 报文已被完整消费
    CHECK(buf.ReadableBytes() == 0);

    std::cout << std::endl;
}

// ---------------------------------------------------------------
// 用例2: POST 请求 + 正文 + 多个查询参数
// ---------------------------------------------------------------
static void TestPostRequest() {
    std::cout << "=== TestPostRequest ===" << std::endl;

    std::string body = "q=hello&x=1";   // 11 字节
    std::string str = "POST /search?q=cpp&page=2&sort=asc HTTP/1.1\r\n"
                      "Host: example.com\r\n"
                      "Content-Type: application/x-www-form-urlencoded\r\n"
                      "Content-Length: 11\r\n"
                      "\r\n" + body;

    HttpContext context;
    Buffer buf;
    buf.set_buf(str);

    CHECK(context.ParseRequest(&buf) == HttpContext::ParseResult::kGotAll);

    const HttpRequest &request = context.request();
    DumpRequest(request);

    CHECK(request.method() == HttpRequest::Method::kPost);
    CHECK(request.version() == HttpRequest::Version::kHttp11);
    CHECK(request.url() == "/search");
    // 多个查询参数，验证 ParseUrl 里 & 的分割
    CHECK(request.request_params().size() == 3);
    CHECK(request.get_request_value("q") == "cpp");
    CHECK(request.get_request_value("page") == "2");
    CHECK(request.get_request_value("sort") == "asc");
    CHECK(request.body() == body);
    CHECK(buf.ReadableBytes() == 0);

    std::cout << std::endl;
}

// ---------------------------------------------------------------
// 用例3: 逐字节分片投喂，验证 kNeedMore 与状态机保持
// ---------------------------------------------------------------
static void TestFragmented() {
    std::cout << "=== TestFragmented ===" << std::endl;

    std::string str = "GET /index.html HTTP/1.1\r\n"
                      "Host: example.com\r\n"
                      "\r\n";

    HttpContext context;
    Buffer buf;

    // 每投喂 1 个字节就尝试解析一次，中途必须一直返回 kNeedMore
    // 注意：真实场景下新到的数据是 Append 到读缓冲区尾部，而不是覆盖
    for (size_t i = 0; i < str.size(); ++i) {
        buf.Append(&str[i], 1);
        HttpContext::ParseResult res = context.ParseRequest(&buf);
        if (i + 1 < str.size()) {
            CHECK(res == HttpContext::ParseResult::kNeedMore);
        } else {
            CHECK(res == HttpContext::ParseResult::kGotAll);
        }
    }

    const HttpRequest &request = context.request();
    DumpRequest(request);
    CHECK(request.method() == HttpRequest::Method::kGet);
    CHECK(request.url() == "/index.html");
    CHECK(request.get_header_value("host") == "example.com");

    std::cout << std::endl;
}

// ---------------------------------------------------------------
// 用例4: 头部与正文分片（Content-Length 未收齐）
// ---------------------------------------------------------------
static void TestFragmentedBody() {
    std::cout << "=== TestFragmentedBody ===" << std::endl;

    std::string head = "POST /submit HTTP/1.1\r\n"
                       "Host: example.com\r\n"
                       "Content-Length: 11\r\n"
                       "\r\n";
    std::string body = "q=hello&x=1";

    HttpContext context;
    Buffer buf;

    // 先投喂整个头部，此时正文一个字节都没到，应等待更多数据
    buf.Append(head.data(), head.size());
    CHECK(context.ParseRequest(&buf) == HttpContext::ParseResult::kNeedMore);

    // 再投喂正文的前 5 个字节，仍然不够
    buf.Append(body.data(), 5);
    CHECK(context.ParseRequest(&buf) == HttpContext::ParseResult::kNeedMore);

    // 投喂剩余字节，解析完成
    buf.Append(body.data() + 5, body.size() - 5);
    CHECK(context.ParseRequest(&buf) == HttpContext::ParseResult::kGotAll);
    CHECK(context.request().body() == body);

    std::cout << std::endl;
}

// ---------------------------------------------------------------
// 用例5: 畸形报文，验证 kError
// ---------------------------------------------------------------
static void TestMalformed() {
    std::cout << "=== TestMalformed ===" << std::endl;

    // 5.1 请求行缺少 HTTP 版本
    {
        HttpContext context;
        Buffer buf;
        buf.set_buf("GET /index\r\n\r\n");
        CHECK(context.ParseRequest(&buf) == HttpContext::ParseResult::kError);
    }

    // 5.2 非法请求方法
    {
        HttpContext context;
        Buffer buf;
        buf.set_buf("FOO /index HTTP/1.1\r\n\r\n");
        CHECK(context.ParseRequest(&buf) == HttpContext::ParseResult::kError);
    }

    // 5.3 非法版本号
    {
        HttpContext context;
        Buffer buf;
        buf.set_buf("GET /index HTTP/2.0\r\n\r\n");
        CHECK(context.ParseRequest(&buf) == HttpContext::ParseResult::kError);
    }

    // 5.4 url 不以 / 开头
    {
        HttpContext context;
        Buffer buf;
        buf.set_buf("GET index HTTP/1.1\r\n\r\n");
        CHECK(context.ParseRequest(&buf) == HttpContext::ParseResult::kError);
    }

    // 5.5 首部行缺少冒号
    {
        HttpContext context;
        Buffer buf;
        buf.set_buf("GET /index HTTP/1.1\r\nHost example.com\r\n\r\n");
        CHECK(context.ParseRequest(&buf) == HttpContext::ParseResult::kError);
    }

    // 5.6 Content-Length 不是数字
    {
        HttpContext context;
        Buffer buf;
        buf.set_buf("POST /index HTTP/1.1\r\nContent-Length: abc\r\n\r\n");
        CHECK(context.ParseRequest(&buf) == HttpContext::ParseResult::kError);
    }

    std::cout << std::endl;
}

// ---------------------------------------------------------------
// 用例6: 首部行的边界情况（空值、空格数量不固定、Tab）
// ---------------------------------------------------------------
static void TestHeaderEdgeCase() {
    std::cout << "=== TestHeaderEdgeCase ===" << std::endl;

    std::string str = "GET / HTTP/1.1\r\n"
                      "Host:example.com\r\n"          // 冒号后 0 个空格
                      "X-Empty:\r\n"                  // 空值
                      "X-Spaces:    a b\r\n"          // 多个空格，值内部保留空格
                      "X-Tab:\tvalue\r\n"             // Tab 分隔
                      "\r\n";

    HttpContext context;
    Buffer buf;
    buf.set_buf(str);

    CHECK(context.ParseRequest(&buf) == HttpContext::ParseResult::kGotAll);

    const HttpRequest &request = context.request();
    DumpRequest(request);

    CHECK(request.get_header_value("host") == "example.com");
    CHECK(request.get_header_value("x-empty").empty());
    CHECK(request.get_header_value("x-spaces") == "a b");
    CHECK(request.get_header_value("x-tab") == "value");

    std::cout << std::endl;
}

// ---------------------------------------------------------------
// 用例7: HTTP/1.0 且无 Host 首部
// ---------------------------------------------------------------
static void TestHttp10() {
    std::cout << "=== TestHttp10 ===" << std::endl;

    HttpContext context;
    Buffer buf;
    buf.set_buf("GET / HTTP/1.0\r\n\r\n");

    CHECK(context.ParseRequest(&buf) == HttpContext::ParseResult::kGotAll);
    CHECK(context.request().version() == HttpRequest::Version::kHttp10);
    CHECK(context.request().version_str() == "HTTP/1.0");
    CHECK(context.request().url() == "/");
    CHECK(context.request().header().empty());

    std::cout << std::endl;
}

// ---------------------------------------------------------------
// 用例8: Reset 后可复用同一个 HttpContext 解析下一个请求
// ---------------------------------------------------------------
static void TestReset() {
    std::cout << "=== TestReset ===" << std::endl;

    HttpContext context;
    Buffer buf;

    buf.set_buf("GET /first HTTP/1.1\r\nHost: a.com\r\n\r\n");
    CHECK(context.ParseRequest(&buf) == HttpContext::ParseResult::kGotAll);
    CHECK(context.request().url() == "/first");

    context.Reset();
    CHECK(!context.GotAll());

    buf.set_buf("GET /second HTTP/1.1\r\nHost: b.com\r\n\r\n");
    CHECK(context.ParseRequest(&buf) == HttpContext::ParseResult::kGotAll);
    CHECK(context.request().url() == "/second");
    CHECK(context.request().get_header_value("host") == "b.com");
    // 上一个请求的残留数据必须被清干净
    CHECK(context.request().get_header_value("host") != "a.com");

    std::cout << std::endl;
}

int main() {
    TestGetRequest();
    TestPostRequest();
    TestFragmented();
    TestFragmentedBody();
    TestMalformed();
    TestHeaderEdgeCase();
    TestHttp10();
    TestReset();

    std::cout << "==============================" << std::endl;
    std::cout << "passed: " << g_pass << ", failed: " << g_fail << std::endl;
    return g_fail == 0 ? 0 : 1;
}
