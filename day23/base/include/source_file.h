#pragma once
#include <cstring>
// 日志尾部要打出文件名，但是_FILE_会导致一次性输出完整的路径名
// 因此需要将文件路径截断成文件名，实现零拷贝保存

class SourceFile{
public:
    explicit SourceFile(const char* filename) : data_(filename){
        const char* slash = strrchr(filename, '/');     // 从后往前，查找filename里第一次出现/的位置，返回该位置的指针
        if(slash != nullptr){
            data_ = slash + 1;    // 将data_更新到截断后的位置，即/之后的位置，对应的就是文件名
        }
        size_ = strlen(data_);
    }

    const char* data() const {return data_;}
    size_t size() const {return size_;}
private:
    const char* data_;  //  指向文件名首字节
    size_t size_;          //  文件名的字符数
};