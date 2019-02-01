/*
Author:			Colin Day
Date:			December 7th 2018
Object:			Given a sample of other playlists, constructed into
				a directory (or 'song matrix') return the most likely
				compatable addition to a given playlist. 
*/
#include <iostream>
#include <fstream>
#include <set>
#include <string>
#include <map>
#include <vector>
#include <ctime>
#include <algorithm>

using std::vector;
using std::pair;
using std::set;
using std::string;
using std::map;

class playlist {
public:
	set<string> songlist{};
	string owner;
	playlist() {
		owner = "not specified";
	}
	playlist(std::ifstream& file) {
		string read;
		while (!file.eof()) {
			getline(file, read);
			if (owner.empty()) {
				owner = read;
			}
			else {
				songlist.insert(read);
			}
		}
	}

	void add(string in) {
		songlist.insert(in);
	}
	double size() {
		return songlist.size();
	}
	bool contains_both(string one, string two) {
		return (songlist.find(one) != songlist.end() && songlist.find(two) != songlist.end());
	}
	void print() {
		std::cout << owner << "'s playlist:" << std::endl;
		for (string song : songlist) {
			std::cout << "- " << song << std::endl;
		}
	}
};

bool operator<(playlist lhs, playlist rhs) {
	return strcmp(lhs.owner.c_str(), rhs.owner.c_str()) < 0;
}

void scale_map(map<string,float>& in) {

	float max = in.begin()->second;
	for (auto f : in) {
		if (f.second > max) max = f.second;
	}
	for (auto f : in)
		f.second /= max;
}

string max_map(const map<string, float>& in) {
	float max = in.begin()->second;
	for (auto f : in) {
		if (f.second > max) max = f.second;
	}
	for (auto f : in) {
		if (f.second == max)
			return f.first;
	}
	throw std::runtime_error("Major booboo");
}

class song_matrix {
public:
	set<playlist> playlists{};
	set<string> all_songs{};
	vector<string> index_all_songs{};
	map<string, unsigned int> instances{};
	map<string, vector<float>> matrix;
	song_matrix(set<playlist> input) {
		//std::cout << "Recieved input of size " << input.size();
		
		
		//----------------STORING PLAYLISTS and ALL SONGS---------------
		playlists = input;
		for (playlist p : input) {
			//std::cout << "Parsing together " << p.owner << std::endl;
			for (string song : p.songlist) {
				all_songs.insert(song);
			}
		}

		//PRINT
		/*for (string s : all_songs)
			std::cout << s <<' ';
		std::cout << std::endl;*/

		//----------------------------COUNTING INSTANCES--------------
		for (string s : all_songs) {
			unsigned int instance_count{};
			for (playlist p : playlists)
				for (string p_s : p.songlist)
					if (p_s == s)
						instance_count++;
			instances[s] = instance_count;
		}

		//PRINT
		/*for (auto i : instances)
			std::cout << i.first << ' ' << i.second << std::endl;
		std::cout << std::endl;*/
		
		
		//CONSTRUCTING THE MATRIX
		for (auto i : instances) {
			string top_song = i.first;
			unsigned int total_count = i.second;
			vector<float> score_vector{};
			for (string side_song : all_songs) {
				float side_song_score{};
				for (playlist p : playlists) 
					if (p.contains_both(top_song, side_song))
						side_song_score++;
				score_vector.push_back(side_song_score/total_count);
			}
			matrix[top_song] = score_vector;
		}

		//CONSTRUCTING THE REFERENCE INDEXABLE MATRIX
		for (auto s : all_songs)
			index_all_songs.push_back(s);
	}

	string recomend(playlist incomplete) {
		map<string, float> scores{};
		for (string song : incomplete.songlist) {
			if (all_songs.find(song) == all_songs.end())
				throw std::runtime_error("Song not in library! No!");
		}
		for (string s : all_songs)
			scores[s] = 0;
		for (string song : incomplete.songlist) {
			unsigned int vector_tracker{};
			for (string s : all_songs) {
				scores[s] += matrix[song].at(vector_tracker);
				vector_tracker++;
			}
		}
		for (string i : incomplete.songlist) 
			scores.erase(i);
		//for (auto i : scores)
			//std::cout << "[" << i.first << "] " << i.second << std::endl;
		return max_map(scores);
	}
	playlist random(unsigned int provide_count) {
		playlist result;
		while (provide_count > 0) {
			int r = rand() % all_songs.size();
			auto it = all_songs.begin();
			std::advance(it, r);
			result.add(*it);
			provide_count--;
		}
		return result;
	}
	void fill_in(playlist& in,unsigned int size) {
		while (in.size() < size) {
			in.add(recomend(in));
		}
	}

};

void txt_fix(string& in) {
	if (in.compare(in.size() - 4, 4, ".txt") != 0)
		in += ".txt";
}

song_matrix directory_construct(string filename) {
	set<playlist> playlists{};
	txt_fix(filename);
	std::ifstream dir(filename);
	while (!dir.eof()) {
		string playfilename{};
		getline(dir, playfilename);
		txt_fix(playfilename);
		std::ifstream playfile(playfilename);
		playlist p(playfile);
		//p.print();
		playlists.insert(p);
	}

	return song_matrix(playlists);
}



int main() {
	srand(time(0));
	auto s = directory_construct("directory");
	
	auto inc = s.random(2);

	inc.print();
	s.fill_in(inc,3);
	inc.print();

	//std::cout << s.recomend(inc) << std::endl;
	return 0;
}