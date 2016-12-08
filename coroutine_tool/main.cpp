
#include "coroutine_tool.h"
#include <tuple>

extern void test();

template<class X>
auto aggg(X x)->decltype(std::get<0>(x.getTup()))
{
	return std::get<0>(x.getTup());
}

struct oo
{

};

typedef std::tuple<oo, int, int> Tup;

template<class TP>
struct llss{
	TP &getTup(){ return tup; }
	TP tup;
};

template<class T>
void ppp(T t)
{
	

	llss<Tup> ls;
	auto x = aggg(ls);
}

int main()
{
	test();

	Tup tup;
	ppp(tup);

	return 0;
}