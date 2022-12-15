#include <iostream>
#include <fstream>
#include <vector>
#include <regex>
#include <iomanip>

using std::cout; using std::cerr;
using std::endl; using std::string;
using std::stringstream; using std::setfill; using std::setw;
using std::ifstream; using std::ofstream;
using std::getline; using std::vector;
using std::ios;
using std::size; 
using std::equal; 
using std::regex; using std::regex_match;

enum lineKbn {
	deflinekbn,
	step,
	space,
	comment,
};

enum lineKbn checkLine(string);
string getStrNowTime();

int main(int argc, char* argv[]){
	// ファイル区分
	const int FILEKBN_ADD = 0;
	const int FILEKBN_MOD = 1;
	const int FILEKBN_DEL = 2;

	const string LINEHEAD_ADDFILE = "+++ ";
	const string LINEHEAD_DELFILE = "--- ";
	const string LINEHEAD_ADDLINE = "+";
	const string LINEHEAD_DELLINE = "-";
	const string LINEHEAD_COUNTLINE = "@@ ";
	const string FILEEND_LINE = "===================================================================";

	string inputfilepath("FILE.TXT");
	string inputfilename("FILE");
	if(argc == 2) {
		inputfilepath = argv[1];
		int path_i = inputfilepath.find_last_of("\\") + 1;
		int ext_i = inputfilepath.find_last_of(".");
		inputfilename = inputfilepath.substr(path_i, ext_i - path_i);
	}
	string outputfilename;
	outputfilename = inputfilename;
	outputfilename += "_DiffCnt_";
	outputfilename += getStrNowTime();
	outputfilename += ".CSV";	
	int fileKbn = -1;
	vector<string> lines;
	// vector<string> tempAddLines;
	// vector<string> tempDelLines;
	string line;
	lineKbn lnKbn = deflinekbn;

	// 出力用変数
	string unitLine;
	string fileKbnName;
	bool isNotAddUnit = false;
	int addStepCnt, addSpaceCnt, addCommentCnt, delStepCnt, delSpaceCnt, delCommentCnt;
	addStepCnt = addSpaceCnt = addCommentCnt = delStepCnt = delSpaceCnt = delCommentCnt = 0;
	int unitAddLineCnt, unitAddStepCnt, unitAddSpaceCnt, unitAddCommentCnt;
	unitAddLineCnt = unitAddStepCnt = unitAddSpaceCnt = unitAddCommentCnt = 0;
	int unitModStepCnt = 0; 
	int unitDelLineCnt, unitDelStepCnt, unitDelSpaceCnt, unitDelCommentCnt;
	unitDelLineCnt = unitDelStepCnt = unitDelSpaceCnt = unitDelCommentCnt = 0;
	int unitDescCnt = 0;
	int unitExtraCnt = 0;
	int totalAddStepCnt, totalModStepCnt, totalDelStepCnt;
	totalAddStepCnt = totalModStepCnt = totalDelStepCnt = 0;

	// test
	// cout << argc << endl;
	// for(int i = 0; i < argc; i++){
	// 	cout << argv[i] << endl;
	// }

	ifstream input_file(inputfilepath);
	ofstream write_file(outputfilename);

	if(!input_file.is_open()){
		cerr << "Could not open the file - '"
			 << inputfilepath << "'" << endl;
		return EXIT_FAILURE;
	}
	if(!write_file.is_open()){
		cerr << "Could not open the file - '"
			 << outputfilename << "'" << endl;
		return EXIT_FAILURE;
	}

	while (getline(input_file, line)){
		lines.push_back(line);
	}
	auto itr = lines.begin();
	while(itr != lines.end()){
		if(((*itr).size() > 4) &&
				 (equal(LINEHEAD_ADDFILE.begin(),LINEHEAD_ADDFILE.end(),(*itr).begin(),(*itr).begin()+4) ||
				  equal(LINEHEAD_DELFILE.begin(),LINEHEAD_DELFILE.end(),(*itr).begin(),(*itr).begin()+4))  ){
			// ファイル名行のとき
			unitLine = (*itr).substr(4);
		}else if((*itr).size() > 0 && equal(LINEHEAD_ADDLINE.begin(),LINEHEAD_ADDLINE.end(),(*itr).begin(),(*itr).begin()+1)){
			// 追加行のとき
			lnKbn = checkLine(*itr);
			switch (lnKbn)
			{
			case step: 
				addStepCnt++; 
				// write_file << (*itr) << endl; 
				break;
			case space: addSpaceCnt++; break;
			case comment: addCommentCnt++; break;
			default: break;
			}
		}else if((*itr).size() > 0 && equal(LINEHEAD_DELLINE.begin(),LINEHEAD_DELLINE.end(),(*itr).begin(),(*itr).begin()+1)){
			// 削除行のとき
			lnKbn = checkLine(*itr);
			switch(lnKbn){
				case step: delStepCnt++; break;
				case space: delSpaceCnt++; break;
				case comment: delCommentCnt++; break;
				default: break;
			}
			isNotAddUnit = true;
		}else if((*itr).size() > 3 && equal(LINEHEAD_COUNTLINE.begin(),LINEHEAD_COUNTLINE.end(),(*itr).begin(),(*itr).begin()+3)){
			// 備考行のチェック
			unitDescCnt++;	// カウント
		}else{
			// 余分な行のとき
			unitExtraCnt++;	// カウント

			// 追加行・削除行のカウント中のとき
			// 2022/12/13 今回は各ステップ数だけカウントするようにする。
			if(addStepCnt > 0 || addCommentCnt > 0 || addSpaceCnt > 0 ){
				if(delStepCnt > 0 || delCommentCnt > 0 || delSpaceCnt > 0){
					if(addStepCnt <= 0){
						unitDelStepCnt += delStepCnt;
					}
					unitModStepCnt += addStepCnt;
				}else{
					unitAddStepCnt += addStepCnt;
				}
			}else if(delStepCnt > 0 || delCommentCnt > 0 || delSpaceCnt > 0){
				unitDelStepCnt += delStepCnt;
			}

			// 一時変数クリア
			addStepCnt = addCommentCnt = addSpaceCnt = delStepCnt = delCommentCnt = delSpaceCnt = 0;
		}
		


		itr++;
		if(itr == lines.end() || equal(FILEEND_LINE.begin(), FILEEND_LINE.end(), (*itr).begin()) ){
			// ファイル区分の分類
			if(unitExtraCnt < 4 && unitDescCnt <2 && unitModStepCnt<=0){
				if(unitAddStepCnt > 0 && unitDelStepCnt <= 0) fileKbn = FILEKBN_ADD;
				else if (unitAddStepCnt <= 0 && unitDelStepCnt > 0) fileKbn = FILEKBN_DEL;
				else fileKbn = FILEKBN_MOD;
			}else{
				fileKbn = FILEKBN_MOD;
			}

			// ファイル区分名付与
			switch(fileKbn){
				case FILEKBN_ADD:
					fileKbnName = "ADD";
					break;
				case FILEKBN_DEL:
					fileKbnName = "DEL";
				case FILEKBN_MOD:
					fileKbnName = "MOD";
					break;
				default:
					break;
			}

			// モジュール名取得できているときのみ出力
			if(unitLine.size() > 0) write_file << fileKbnName << "," << unitLine << "," << unitAddStepCnt << "," << unitModStepCnt << "," << unitDelStepCnt << "," << endl;

			totalAddStepCnt += unitAddStepCnt;
			totalModStepCnt += unitModStepCnt;
			totalDelStepCnt += unitDelStepCnt;

			// 初期化
			fileKbn = -1;
			unitLine = "";
			fileKbnName = "";
			isNotAddUnit = false;
			unitAddLineCnt = 0;
			unitAddStepCnt = 0;
			unitAddSpaceCnt = 0;
			unitAddCommentCnt = 0;
			unitModStepCnt = 0;
			unitDelLineCnt = 0;
			unitDelStepCnt = 0;
			unitDelSpaceCnt = 0;
			unitDelCommentCnt = 0;
			unitDescCnt = 0;
			unitExtraCnt = 0;
		}
	}
	write_file << " ,SUM," << totalAddStepCnt << "," << totalModStepCnt << "," << totalDelStepCnt << endl;

	write_file.close();
	input_file.close();
	return EXIT_SUCCESS;
}

enum lineKbn checkLine(string line){
	regex reComment(R"([+-]\s*'.*)");
	regex reSpace(R"([+-]\s*)");
	if(regex_match(line, reComment)){
		return comment;
	}else if(regex_match(line, reSpace)){
		return space;
	}else{
		return step;
	}
}

string getStrNowTime(){
	time_t nowtime = time(nullptr);	// 現在時刻取得
	tm* nt = localtime(&nowtime);	// 形式変換
	stringstream s;
	s << "20" << setfill('0') << setw(2) << nt -> tm_year-100;
	s << setfill('0') << setw(2) << nt -> tm_mon + 1;
	s << setfill('0') << setw(2) << nt -> tm_mday;
	s << setfill('0') << setw(2) << nt -> tm_hour;
	s << setfill('0') << setw(2) << nt -> tm_min;
	s << setfill('0') << setw(2) << nt -> tm_sec;
	return s.str();
}