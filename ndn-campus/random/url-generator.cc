/*
 *
 * url-generator.cc
 *
 *  Simple command line program to generate a random list of URLs given an
 *  amount of desired URLs, a mean and standard deviation for their lengths and
 *  a dictionary file.
 *
 *  Created on: June 24, 2014
 *      Author: Jairo Eduardo Lopez
 */
#include <algorithm>
#include <ctime>
#include <iostream>
#include <iterator>
#include <fstream>
#include <sys/stat.h>
#include <vector>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/geometric_distribution.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/program_options.hpp>

using namespace boost::random;
using namespace std;
namespace po = boost::program_options;

mt19937_64 gen;

struct length {
	bool operator() ( const string& a, const string& b )
	{
		return a.size() < b.size();
	}
};

int main(int ac, char* av[])
{
	gen.seed(std::time(0) + (long long)getpid() << 32);
	po::variables_map vm;

	try {

		po::options_description desc("Allowed options");
		desc.add_options()
	            		("help", "Produce this help message")
	            		("avg", po::value<int>(), "Set average length of the URL")
	            		("std", po::value<int>(), "Set URL length standard deviation")
	            		("num", po::value<int>(), "Set number of URLs to generate")
	            		("file", po::value<string>(), "Dictionary file")
	            		("out", po::value<string>(), "Output file for generated data")
	            		;

		po::store(po::parse_command_line(ac, av, desc), vm);
		po::notify(vm);

		if (vm.count("help")) {
			cout << desc << endl;
			return 0;
		}

		if (! vm.count("avg")) {
			cout << "URL avg length not set!." << endl;;
			return 1;
		}

		if (! vm.count("std")) {
			cout << "URL standard deviation length not set!." << endl;;
			return 1;
		}

		if (! vm.count("num")) {
			cout << "Number of URLs to generate not set!." << endl;;
			return 1;
		}

		if (! vm.count("file")) {
			cout << "Dictionary file not set!." << endl;
			return 1;
		}

		if (! vm.count("out")) {
			cout << "Output file not set!." << endl;
			return 1;
		}
	}
	catch(std::exception& e) {
		cerr << "error: " << e.what() << endl;
		return 1;
	}
	catch(...) {
		cerr << "Exception of unknown type!" << endl;
	}

	vector<string> list;

	string filename = vm["file"].as<string>();
	string outputfile = vm["out"].as<string>();
	int numURLs = vm["num"].as<int>();
	int mean = vm["avg"].as<int>();
	int std = vm["std"].as<int>();

	// Try opening the dictionary file
	ifstream in_stream(filename.c_str());

	if (in_stream)
	{
		string line;

		while(!in_stream.eof())
		{
			in_stream >> line;
			list.push_back(line);
		}
	}
	else
	{
		cout << "File " << filename << " Does not exist!" << endl;
		return 1;
	}

	// Close the dictionary file
	in_stream.close();

	// Sort the vector by the longest string
	sort ( list.begin(), list.end(), length());

	int longestStr = list.back().size();
	int dictSize = list.size()-1;

	int minURLlen = longestStr;
	int maxURLlen = mean + std;

	cout << "Longest string given was: " << longestStr << endl;
	cout << "Size of dictionary is: " << dictSize << endl;
	cout << "Generating " << numURLs << " URLs" << endl;

	// Random to get the size of the URL to produce
	uniform_int_distribution<> url_len(minURLlen, maxURLlen);

	variate_generator<mt19937_64&, uniform_int_distribution<> > url_size_gen(gen, url_len);

	// Random to get the a string from the vector we created
	uniform_int_distribution<> dict_num(0, dictSize);

	variate_generator<mt19937_64&, uniform_int_distribution<> > dict_var_gen(gen, dict_num);

	// Try to open our output file for appending
	ofstream fout(outputfile.c_str(), ios::app);

	if (!fout)
	{
		cout << " Could not open " << outputfile << "for writing...\n";
		return(1);
	}

	for (int i = 0; i < numURLs; i++)
	{
		// Generate a URL length
		int url_curr_len = url_size_gen();

		//cout << "URL size will start as " << url_curr_len << endl;

		string tmp = "";

		while (url_curr_len > minURLlen)
		{
			// Generate a number to choose a string
			int dict_curr_var = dict_var_gen();

			// Add the new string to final result
			tmp += "/" + list[dict_curr_var];

			// Subtract the string length and head for next round
			url_curr_len -= (list[dict_curr_var].size());

			//cout << "Added " << tmp << endl;
			//cout << "We have " << url_curr_len << " characters remaining" << endl;
		}

		//cout << "Resulting string " << tmp << endl;

		// With the string created, we send it to the file
		 fout << tmp << endl;
	}

	// Close the dictionary file
	fout.close();

	return 0;
}
