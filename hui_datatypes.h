#if !defined(HUI_DATATYPES_H)
 #define HUI_DATATYPES_H
 
 // TODO: refactor project code and get rid of this file
 
 #include <string>
 #include <vector>
 #include <iostream>
 
 #if defined(_WIN32) // windows
  #include <Windows.h>
 #else
  #include <unistd.h>
 #endif
 
/*
#define HUI_PROPERTY (name, type, setter, getter, changecallbacksetter) \
	void set_#name# (#type#& #name#) setter\
	#type#& get_#name# () getter
*/

#define HUI_PROPERTY(/*TYPE,*/ NAME, SETTER, GETTER/*, CALLBACK*/) \
	void set_##NAME(/*TYPE*/auto NAME)SETTER \
	/*TYPE*/auto get_##NAME()GETTER 
	/*void get_##NAME(TYPE& _return_value){ _return_value = [&]GETTER(); } \ 
	void get_##NAME(TYPE* _return_value){ (*_return_value) = [&]GETTER(); }*/

/*HUI_PROPERTY(
	testprop, 
	{
		std::cout<<"_set_"<<testprop;
	},
	{
		std::cout<<"_get_";
		return 0;
	}
)*/
 

 
namespace HUI {
	
	
	
template<typename AUTO/*, class ALLOC = allocator<AUTO> */> class Vec : public std::vector<AUTO/*, ALLOC*/> {
	// currently todo
  public:
	void operator+= (AUTO value) { std::vector<AUTO>::push_back(value); }
	void operator+= (Vec value) { for (auto i : value) std::vector<AUTO>::push_back(i); }
};


 
// should not use cstring/const char* - its way more unsafe 
// TODO: stream operators, ==, find by {list, of, substrings}, antifind
class Str : private std::string {
  private:
	//using std::string::operator[];
	
  public: 
    
	using std::string::string;
	
	//Str (auto value) : std::string (value) {} 
	Str (std::string value)        : std::string (value) {}
	Str (const char value)         : std::string ({value}) {}
	Str (int value)                : std::string (std::to_string(value)) {} 
	Str (long value)               : std::string (std::to_string(value)) {} 
	Str (long long value)          : std::string (std::to_string(value)) {} 
	Str (unsigned value)           : std::string (std::to_string(value)) {}
	Str (unsigned long value)      : std::string (std::to_string(value)) {}
	Str (unsigned long long value) : std::string (std::to_string(value)) {}
	Str (float value)              : std::string (std::to_string(value)) {}
	Str (double value)             : std::string (std::to_string(value)) {}
	Str (long double value)        : std::string (std::to_string(value)) {}
	Str (const std::initializer_list<const Str> list, Str between=""){ // python join alternative
		operator+= ( (*list.begin())/*.cpp_str()*/ );
		for (auto it=list.begin()+1; it!=list.end(); ++it){
			operator+= ( between/*.cpp_str()*/ );
			operator+= ( (*it)/*.cpp_str()*/ );	
		}
	}
	Str (const Vec<Str> list, Str between=""){ // once more because cpp needs it
		operator+= ( (*list.begin())/*.cpp_str()*/ );
		for (auto it=list.begin()+1; it!=list.end(); ++it){
			operator+= ( between/*.cpp_str()*/ );
			operator+= ( (*it)/*.cpp_str()*/ );	
		}
	}
	
	//using std::string::operator<<; // not member
	//using std::string::operator>>; // not member
	using std::string::operator=;
	//void operator= (const char str){...}
	using std::string::operator+=;
	
	using std::string::operator[];
	/*char operator[] (size_t index) const {
		if (index >= size()) index = size()-1; // out of range handling
		return at(index);
	}*/
	// Str operator[] (size_t from=0, size_t to=-1) {return Str (part(from,to));} // ready for C++23
	
	//operator std::string() const {return cpp_str();}
	//operator const std::string&() const {return cpp_str();}
	
	operator int() const {try {return std::stoi(c_str());} catch(...) {return 0;} }
	operator long() const {try {return std::stol(c_str());} catch(...) {return 0;} }
	operator long long() const {try {return std::stoll(c_str());} catch(...) {return 0;} }

	using std::string::begin;
	using std::string::end;
	using std::string::cbegin;
	using std::string::cend;
	using std::string::rbegin;
	using std::string::rend;
	using std::string::crbegin;
	using std::string::crend;
	
	operator std::string () {return *this;}
	operator std::string &() {return *this;}
	std::string cpp_str() const {return *this;} // for compatibility but no need to use this (instead std::string str = HUI::Str("abc"); can be used)  // TODO: fix this using public inheritance
	using std::string::c_str;
	
	using std::string::size;
	

	
	//char part (size_t index) {return data.at(index);} // ready for time when return type overloading becomes possible
	//void part (...) {...} // change its the value 
	Str part(size_t from=0, size_t to=-1) const { // can be reversed with from > to ; npos is -1, last is -1
		if (from > size()) from = size(); // out of range handling
		if (from <= to or to == -1) return Str(std::string::substr(from,to-from));
		else return Str( std::string(rbegin(), rend()).substr(size()-from-1, from-to) );
	}

	void insert (Str what, size_t where = -1, size_t eat=0) { //works like append, insert, replace
		if (where >= size()) std::string::append(what);
		else if (eat == 0) std::string::insert(where, what);
		else std::string::replace(where, eat, what);
	}
	// Str insert (...) {...}
	
	size_t find (Str what, size_t from=0, size_t to=-1) const { // search in part - the from and to work the same as in part (from is included, to is not included) 
		if (to > size()) to = size();
		size_t found;
		if (from <= to) {
			found = std::string::find(what, from);
			if (found >= to and to != size_t(-1)) found = -1;
		}
		else {
			found = std::string::rfind(what,from-what.size()+1);
			if (found <= to and to != size_t(-1)) found = -1;
		}
		return found;
	}
	//size_t find(vector<Str> what, size_t from=0, size_t to=-1, bool antifind=false)
	// vector<size_t> find
	
	Str replace (Str old_str, Str new_str) const {
		
		Str return_str;
		size_t level = 0; // position in old_str
		size_t startp = 0; // first position after last replace
		
		for (size_t i = 0; i<size(); i++){
			if (operator[](i) == old_str[level]){
				if (level == old_str.size()-1){
					
					return_str += std::string::substr(startp, i-startp-level); // second is len not pos
					return_str += new_str;
					
					startp = i+1;
					level = 0;
				}
				else level++;
				
			}
			else {
				level=0;
			}
		}
		
		return_str += std::string::substr(startp);
		return return_str;
	}
	//void replace (Vec<Str> old, HTUIstring new){}
	//void replace (std::map<Str,Str> pairs){} // encrypt by key -> can be used to do uppercase/lowercase conversion

	Vec<Str> split (Str separator) const {
		
		Vec<Str> return_vec;
		size_t level = 0; // position in old_str
		size_t startp = 0; // first position after last replace
		
		for (size_t i = 0; i<size(); i++){
			if (operator[](i) == separator[level]){
				if (level == separator.size()-1){
					
					return_vec.push_back(std::string::substr(startp, i-startp-level)); // second is len not pos
					
					startp = i+1;
					level = 0;
				}
				else level++;
				
			}
			else {
				level=0;
			}
		}
		
		return_vec.push_back(substr(startp));
		return return_vec;
	}
	

 };

Str operator+(const char* a, Str b){
	return Str({a,b});
}
Str operator+(Str a, const char* b){
	return Str({a,b});
}
Str operator+(Str a, Str b){
	return Str({a,b});
}
std::ostream& operator<<(std::ostream& stream, Str& s){
    return (stream << s.cpp_str());
}
std::istream& operator>>(std::istream& stream, Str& s){
    std::string ss;
	stream>>ss;
	s = ss;
	return stream;
} 




//const char[] CURRENT_DIR = // TODO: constants/preload
//const char[] EXECUTABLE_DIR =  program_invocation_name
//const char[] EXECUTABLE_NAME = 

#if defined(_WIN32) // windows
Str filepath(Str fn){ // TODO: as fix for msys2-mingw-qt there is '/' before disk letter -> "/C:/Users/..."  +  complete
	fn = fn.replace("\\","/");
	fn = fn.replace("%userprofile%",getenv("userprofile"));
	if (fn.find("/") == -1){
		char invocation_c[MAX_PATH];
		DWORD invocation_l = GetModuleFileName(nullptr, invocation_c, MAX_PATH);
		Str invocation_s = Str(std::string(invocation_c,invocation_l)).replace("\\","/");
		return "/"+invocation_s.part(0,invocation_s.find("/", -1, 0)+1)+fn;
	}
	else if (fn[0] == '.' and fn[1] == '/'){
		char current_c[MAX_PATH];
		DWORD current_l = GetCurrentDirectory(MAX_PATH, current_c);
		Str current_s = Str(std::string(current_c,current_l)).replace("\\","/");
		std::cout<<std::string(current_c,current_l);
		return "/"+current_s+fn.part(1);
	}
	else if (fn[0] == '.' and fn[1] == '.' and fn[2] == '/')  return "";
	else  return fn;	
}
#else // linux
Str filepath(Str fn){
	fn = fn.replace("\\","/");
	fn = fn.replace("$HOME",getenv("HOME"));
	if (fn.find("/") == -1){
		//return Str(program_invocation_name).part(0,Str(program_invocation_name).find(-1))+fn;

		pid_t pid = getpid();
		char buf[20] = {0};
		sprintf(buf,"%d",pid);
		Str _link = Str({"/proc/",buf,"/exe"});
		char proc[512];
		int ch = readlink(_link.c_str(),proc,512);
		if (ch != -1) {
			proc[ch] = 0;
			Str path = proc;
			return path.part(0,path.find("/", -1, 0)+1)+fn;
		}
		return "";
	}
	else if (fn[0] == '.' and fn[1] == '/')  return Str(get_current_dir_name())+fn.part(1);
	else if (fn[0] == '.' and fn[1] == '.' and fn[2] == '/')  return "";
	else  return fn;
}
#endif

};
#endif // HUI_DATATYPES_H