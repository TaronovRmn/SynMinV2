// SynMinV2.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>

using namespace std;

string const SOLS_FILE_NAME = "ouR.txt";
string const REF_FILE_NAME = "enU.txt";
string const TEMP_FILE_NAME = "tmp.txt";

string METHOD_NAME = "[SYN]";

int const REFERENCE_METHOD = 6;
bool const REFERENCE_FILE = true; //if true program will use enU file as reference instead of method

class Solution //solution with: letters, name, index
{
public:
	void setLetters(string & s)
	{
		letters_ = s;
	}

	void setName(string & name)
	{
		if (name.length() == 6) name_ = name;
		else if (name.length() == 5) name_ = name.insert(0, " ");
	}
	void setIndex(int i) { index_ = i; }
	void setTreePosition(int i) { treePosition_ = i; }
	int getSize() const { return letters_.length(); }
	string getLetters() const { return letters_; }
	void setPriority(bool b) { priority_ = b; }
	bool getPriority() const { return priority_; }

	void print(ofstream & fout) const
	{
		fout << letters_ << '\t' << index_ << '\t' << treePosition_ << '\t' << name_ << endl;
	}
	void read(ifstream & fin)
	{
		fin >> letters_;
		fin >> index_;
		priority_ = index_ == REFERENCE_METHOD;
		fin >> treePosition_;
		string name;
		fin >> name;
		this->setName(name);
	}
	void incErrors() { errors_++; }
	bool isError(int i) const { return errorPosition_[i]; }
	friend bool operator== (const Solution &s1, const Solution &s2);
	friend bool operator> (const Solution &s1, const Solution &s2);
	friend bool operator<= (const Solution &s1, const Solution &s2);

	friend bool operator< (const Solution &s1, const Solution &s2);
	friend bool operator>= (const Solution &s1, const Solution &s2);
	void compare(const Solution &rs)
	{
		for (int i = 0; i < letters_.length(); i++)
		{
			if (letters_[i] != rs.getLetters()[i])
			{
				this->incErrors();
				errorPosition_[i] = true;
			}
		}
	}
	static bool advCompare(const Solution &a, const Solution &b) { //if a has less errors than b true
		double aScore = 0;
		double bScore = 0;
		int fl = 0, fr = 0;
		int const len = a.getLetters().length();
		for (int i = 0; i < len; i++)
		{
			if (a.isError(i))
			{
				if (fl == 0) fl = i + 1;
				aScore += log(len - i);
			}
			if (b.isError(i))
			{
				if (fr == 0) fr = i + 1;
				bScore += log(len - i);
			}
		}
		if (aScore < bScore) return false;
		if (aScore == bScore) { if (fl >= fr) return false; }
		return true;
	}
private:
	string letters_;
	int index_ = 0;
	int treePosition_ = 0;
	string name_;
	bool priority_ = false;
	int errors_ = 0;
	bool errorPosition_[40] = {false};
};

bool operator== (const Solution &s1, const Solution &s2)
{
	return s1.errors_ == s2.errors_;
}

bool operator> (const Solution &s1, const Solution &s2)
{
	return s1.errors_ > s2.errors_;
}

bool operator<= (const Solution &s1, const Solution &s2)
{
	return s1.errors_ <= s2.errors_;
}

bool operator< (const Solution &s1, const Solution &s2)
{
	return s1.errors_ < s2.errors_;
}

bool operator>= (const Solution &s1, const Solution &s2)
{
	return s1.errors_ >= s2.errors_;
}

Solution reference;

class Fragment
{
public:
	Fragment(int & k) : numberOfMethods_(k)
	{
		solutions_.reserve(k);
	}
	void read(ifstream & fin)
	{
		if (numberOfMethods_ == 0) {
			cout << "trying to input zero methods";  return;
		}
		string s;
		fin >> no_;
		for (int iS = 0; iS < numberOfMethods_; iS++)
		{
			Solution sol;
			sol.read(fin);
			if (REFERENCE_FILE) refSolution = reference;
			if (sol.getPriority() && !REFERENCE_FILE) refSolution = sol;
			else solutions_.push_back(sol);
		}

	}
	void print(ofstream & fout)
	{
		fout << no_ << endl;
		for (auto & i : solutions_) {
			i.print(fout);
		}
	}
	void findSynMin()
	{
		for (auto & sol :solutions_)
		{
			sol.compare(refSolution);
		}

		const auto bestSolution = min_element(solutions_.begin(), solutions_.end(),
			[] (const Solution & a, const Solution & b)
		{
			if (a == b)
			{
				return Solution::advCompare(a, b);
			}
			return a < b;
		});
		Solution synMinSolution = *bestSolution;
		synMinSolution.setIndex(numberOfMethods_ + 1);
		synMinSolution.setTreePosition(0);
		synMinSolution.setName(METHOD_NAME);
		if (!REFERENCE_FILE) solutions_.push_back(refSolution);
		solutions_.push_back(synMinSolution);
	}
	
private:
	int numberOfMethods_ = 0;
	string no_;
	vector<Solution> solutions_;
	Solution refSolution;
};

void copy_file(ifstream &f, ofstream &t)
{
	string const s((istreambuf_iterator<char>(f)),
		istreambuf_iterator<char>());
	t << s;
}

Solution readRefSolution(ifstream & fin)
{
	Solution ref;
	string s, s1;
	int l, k;
	fin >> l >> k;
	for (int i = 0; i < l; i++) { fin >> s1; s += s1; }
	ref.setLetters(s);
	return ref;

}



int main()
{
	setlocale(LC_ALL, "rus");

	ifstream fin;
	ofstream fout;

	if (REFERENCE_FILE)
	{
		fin.open(REF_FILE_NAME, ios_base::in);
		if (!fin.is_open())
			return 1;
		reference = readRefSolution(fin);
		fin.close();
	}

	fin.open(SOLS_FILE_NAME, ios_base::in);
	if (!fin.is_open())
		return 1;

	int nFrags, nMethods;

	fin >> nFrags >> nMethods;

	vector<Fragment> fragments;
	fragments.reserve(nFrags);
	for (int iF = 0; iF < nFrags; iF++)
	{
		Fragment frag(nMethods);
		frag.read(fin);
		frag.findSynMin();
		fragments.push_back(frag);
	}

	fout.open(TEMP_FILE_NAME, ios_base::out);
	fout << nFrags << " " << nMethods + 1 << endl;
	for (auto & f : fragments)
	{
		f.print(fout);
	}
	fin.close();
	fout.close();
	ifstream fint(TEMP_FILE_NAME);
	if (!fint.is_open())
		return 1;
	ofstream foutt(SOLS_FILE_NAME);

	copy_file(fint, foutt);

	foutt.close();
	fint.close();
	remove(TEMP_FILE_NAME.c_str());
    return 0;
}

