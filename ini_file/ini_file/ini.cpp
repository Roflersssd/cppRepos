#include "ini.h"

namespace Ini{

	Section& Document::AddSection(string name) {
		return sections[name];
	}

	const Section& Document::GetSection(const string& name) const {
		return sections.at(name);
	}

	size_t Document::SectionCount() const {
		return sections.size();
	}

	pair<string, string> parseLine(const string& line, const char& sep) {
		size_t npos = line.find(sep);
		pair<string, string> res(line.substr(0, npos), line.substr(npos + 1, line.size()));
		return res;
	}

	Document Load(istream& input){
		Document doc;

		Section* sectionPtr=nullptr;
		char c;
		while (input >> c) {		
			if (c == '[') {
				string section;
				getline(input, section, ']');
				sectionPtr=&doc.AddSection(section);
			}
			else {
				string line;
				input.putback(c);
				getline(input, line);
				sectionPtr->insert(parseLine(line, '='));
			}
		}
		return doc;
	}
}