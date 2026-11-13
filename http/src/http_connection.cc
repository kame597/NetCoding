#include "http_connection.h"

HttpConnection::HttpConnection(const std::shared_ptr<TcpConnection> &conn):
    conn_(conn)
{}

HttpConnection::~HttpConnection()
{}

void HttpConnection::OnMessage(Buffer *buf){
    while(true){    //此处循环是为了避免粘包问题，处理成功一个后会接着尝试处理，直到无法处理
        HttpContext::ParseResult result = context_.ParseRequest(buf);    
        if(result == HttpContext::ParseResult::kGotAll){
            HandleRequest(context_.request());
            context_.Reset();
            if(should_close_) break;
        }
        else if(result == HttpContext::ParseResult::kError){
            should_close_ = true;
            break;
        }
        else    //needmore，避免半包问题
            break;  
    }
    if(should_close_)   // 如果发现报文错误，则调用tcp连接的关闭回调，关闭连接
        conn_->HandleClose();
    return;
}

void HttpConnection::set_message_callback(MessageCallback cb){
    message_callback_ = std::move(cb);
}

const std::shared_ptr<TcpConnection> &HttpConnection::tcp_conn() const{
    return conn_;
}


int HttpConnection::keepalive_count() const{
    return keepalive_count_;
}



bool HttpConnection::should_close() const{
    return should_close_;
}

/*
*   HandleRequest
*   创建一个HttpResponse，交付给message_callback_填装
*   填装完成后，交付给SendResponse，进行序列化发送
*/
void HttpConnection::HandleRequest(const HttpRequest &req){
    HttpResponse resp;
    if(message_callback_){
        message_callback_(req, &resp);
        SendResponse(resp);
    }
}

/*
*   SendResponse
*   通过TcpConnection，发送序列化了的HttpResponse
*/
void HttpConnection::SendResponse(const HttpResponse &resp){
    std::string std_resp = resp.ToString(true);
    conn_->Send(std_resp);
}

// void HttpConnection::Reset(){
//     context_.Reset();
// }
