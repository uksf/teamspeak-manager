#pragma once

template<class T> class Singleton {
public:
	static T* getInstance() {
		static T obj;

		return &obj;
	}

//private:
//	//T() {}
//
//public:
//	Singleton(Singleton const&) = delete;
//	void operator=(Singleton const&) = delete;
};

