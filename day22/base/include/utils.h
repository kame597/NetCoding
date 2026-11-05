#pragma once

/*
*   @brief
*   条件成立时输出 FATAL 日志并终止进程（内部走 LOG_FATAL，abort 退出）
*   注：errmsg 之后会自动追加 ": " + strerror(errno)，
*       调用方无需自行拼接 errno 信息。
*/
void errif(bool condition, const char *errmsg);
