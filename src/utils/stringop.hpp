#include <stdio.h>
#include <string>
#include <vector>
/*
名称:my_split(const std::string& src, const char& delim,
         std::vector<std::string>& vec)

功能:用分隔符将源字符串分隔为多个子串并传出; n个分隔符, 分n+1个子串
    

参数:
     src-传入参数, 源字符串;
     delim-传入参数, 分隔符;
     vec-传出参数, 子串的集合;
  
返回值:
    0-成功;
    其它-失败;
 */

int string_split(const std::string& src, const char& delim,
		std::vector<std::string>& vec)
{
	int src_len = src.length();
	int find_cursor = 0;
	int read_cursor = 0;

	if (src_len <= 0) return -1;

	vec.clear();
	while (read_cursor < src_len){

		find_cursor = src.find(delim, find_cursor);

		//1.找不到分隔符
		if (-1 == find_cursor){
			if (read_cursor <= 0) return -1;

			//最后一个子串, src结尾没有分隔符
			if (read_cursor < src_len){
				vec.push_back(src.substr(read_cursor, src_len - read_cursor));
				return 0;
			}
		}
		//2.有连续分隔符的情况
		else if (find_cursor == read_cursor){
			//字符串开头为分隔符, 也按空子串处理, 如不需要可加上判断&&(read_cursor!=0)
			vec.push_back(std::string(""));
		}
		//3.找到分隔符
		else
			vec.push_back(src.substr(read_cursor, find_cursor - read_cursor));

		read_cursor = ++find_cursor;
		if (read_cursor == src_len){
			//字符串以分隔符结尾, 如不需要末尾空子串, 直接return
			vec.push_back(std::string(""));
			return 0;
		} 
	}//end while()

	return 0;
}