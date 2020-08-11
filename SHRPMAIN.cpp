//#include <algorithm>
#include <unistd.h>
#include <sys/stat.h>
#include <string>
//#include <sstream>
#include <fstream>
#include "twcommon.h"
#include "variables.h"
#include "twrp-functions.hpp"
#include "data.hpp"
#include "gui/gui.hpp"

#include <openssl/sha.h>	// sha hashing
#include <iomanip>		// setw for hashing
#include <random>		// salting

#include "SHRPMAIN.hpp"
#include "SHRPTOOLS.hpp"

//Text Editor Funcs()
int textEditor::getLineNo(string path){
	int line=1;
	string tmp;
	fstream file;
	file.open(path.c_str(),ios::in);
	if(file){
		while(getline(file, tmp)){
			line++;
		}
	}else{
		return 0;
	}
	file.close();
	return line-1;
}

string textEditor::handleTab(string str){
	int tmp1=str.find_first_of('\t');
	while(tmp1!=-1){
		str[tmp1]=' ';
		str.insert(tmp1,"  ");
		tmp1=str.find_first_of('\t');
	}
	return str;
}

void textEditor::disp_file(string path){
	fstream file;
	string tmp;
	int line_no=1;
	file.open(path.c_str(),ios::in);
	if(file){
		tmp="Text File - "+path;
		gui_msg(Msg(tmp.c_str(),0));
		while(getline(file, tmp)){
			{
      	stringstream guun;
        string t;
        guun<<line_no;
        guun>>t;
				tmp=handleTab(tmp);
        tmp=t+" "+tmp;
				gui_msg(Msg(tmp.c_str(),0));
      }
      line_no++;
    }
	}
	file.close();
}

void textEditor::getdString(string path,string &text1,string &text2,int line,int arg){
	fstream file;
	string tmp;
	int line_no=1;
	int fileLineNo=getLineNo(path);
	int swt=1;
	if(line>fileLineNo){
		swt=0;
	}else if(line<line_no){
		line=1;
	}
	file.open(path.c_str(),ios::in);
	if(file&&swt){
		while(getline(file,tmp)){
			if(line_no<line){
				text1+=tmp+"\n";
			}
			if(arg==1&&line_no==line){
				text2+=tmp+"\n";
			}
			if(line_no>line){
				text2+=tmp+"\n";
			}
			line_no++;
		}
	}else if(file){
		while(getline(file,tmp)){
			text1+=tmp+"\n";
		}

	}
	file.close();
}

void textEditor::getReplacebleLine(string path,int line){
	fstream file;
	string tmp;
	int line_no=1;
	if(line<=line_no){
		line=1;
	}
	file.open(path.c_str(),ios::in);
	if(file){
		while(getline(file,tmp)){
			if(line_no==line){
				break;
			}
			line_no++;
		}
		if(line_no<line){
				tmp=" ";
		}else{
				tmp=handleTab(tmp);
		}
		DataManager::SetValue("replaceText",tmp.c_str());
	}
	file.close();
}

void textEditor::pushString(string path,string text){
	fstream file;
	file.open(path.c_str(),ios::out);
	file<<text;
	file.close();
}

void textEditor::replaceLine(string path,string rtext,int line){
	string up,down;
	getdString(path,up,down,line,0);
	rtext=up+rtext+"\n"+down;
	pushString(path,rtext);
}

void textEditor::addLine(string path,string rtext,int line){
	string up,down;
	getdString(path,up,down,line,1);
	rtext=up+rtext+"\n"+down;
	pushString(path,rtext);
}

void textEditor::removeLine(string path,int line){
	string up,down;
	getdString(path,up,down,line,0);
	up=up+down;
	pushString(path,up);
}



//Theme Parser
void ThemeParser::pushValues(){
	DataManager::SetValue("c_white",bgColor.c_str());
	DataManager::SetValue("nav_bg",navBgColor.c_str());
	DataManager::SetValue("c_black",textColor.c_str());
	DataManager::SetValue("c_acc_color_val",accColor.c_str());
}
void ThemeParser::processValue(string line,int arg){
	line=line.substr(line.find_last_of("=")+1,line.length());
	switch(arg){
		case 0:themeName=line;
		break;
		case 1:bgColor=line;
		break;
		case 2:navBgColor=line;
		break;
		case 3:accColor=line;
		break;
		case 4:textColor=line;
		break;
	}
	LOGINFO("SHRP THEME PARSHER: %s\t %s\t %s\t %s\t %s\n",themeName.c_str(),bgColor.c_str(),navBgColor.c_str(),accColor.c_str(),textColor.c_str());
}
void ThemeParser::fetchInformation(string path){
	string tmp;
	fstream file;
	LOGINFO("SHRP THEME PARSHER: Theme Path %s\n",path.c_str());
	file.open(path.c_str(),ios::in);
	if(file){
		int i=0;
		while(getline(file,tmp)){
			processValue(tmp,i++);
		}
	}
}
bool ThemeParser::verifyColor(string arg){
	int tmp=arg.length();
	if((tmp==7||tmp==9)&&arg[0]=='#'){
	}else{
		LOGINFO("SHRP THEME PARSHER: Incorrect Hex Format..(#)\n");
		return false;
	}
	tmp--;
	//tmp=-2;
	while(tmp!=0){
		if(!(arg[tmp]=='1'||arg[tmp]=='2'||arg[tmp]=='3'||arg[tmp]=='4'||arg[tmp]=='5'||arg[tmp]=='6'||arg[tmp]=='7'||arg[tmp]=='8'||arg[tmp]=='9'||arg[tmp]=='0'||arg[tmp]=='A'||arg[tmp]=='a'||arg[tmp]=='B'||arg[tmp]=='b'||arg[tmp]=='C'||arg[tmp]=='c'||arg[tmp]=='D'||arg[tmp]=='d'||arg[tmp]=='E'||arg[tmp]=='e'||arg[tmp]=='F'||arg[tmp]=='f')){
			LOGINFO("SHRP THEME PARSHER: Incorrect Hex Format..(value)\n");
			return false;
		}
		tmp--;
	}
	return true;
}

bool ThemeParser::verifyInformation(){
	LOGINFO("SHRP THEME PARSHER: Verifing theme data..\n");
	if(verifyColor(bgColor)&&verifyColor(accColor)&&verifyColor(textColor)){
		return true;
	}else{
		return false;
	}
}

//JSON_Genarator
string JSON::getVar(string var,string val){
	char exp='"';
	if(val=="true"||val=="false"){
		return exp+var+exp+": "+val;
	}else{
		return exp+var+exp+": "+exp+val+exp;
	}
}
string JSON::getVar(string var,int val){
	char exp='"';
	return exp+var+exp+": "+to_string(val);
}
string JSON::getVar(string var,float val){
	char exp='"';
	return exp+var+exp+": "+to_string(val);
}
string JSON::genarateRAWJson(){
#ifdef SHRP_BUILD_DATE
	string build;
	stringstream date(EXPAND(SHRP_BUILD_DATE));
	date>>build;
#else
	string build="none";
#endif
#ifdef SHRP_EXPRESS
	string express="true";
#else
	string express="false";
#endif
	return getVar("codeName",DataManager::GetStrValue("device_code_name"))+","+getVar("buildNo",build)+","+getVar("isOfficial",DataManager::GetStrValue("is_Official"))+","+getVar("has_express",express)+","+getVar("shrpVer",DataManager::GetStrValue("shrp_ver"));
}

void JSON::storeShrpInfo(){
	if(DataManager::GetIntValue(TW_IS_ENCRYPTED)==0){
		string text="[{"+genarateRAWJson()+"}]";
		//Creating the folder
		TWFunc::Exec_Cmd("mkdir -p /data/shrp/",true);
		//Pushing the json
		fstream file;
		file.open("/data/shrp/shrp_info.json",ios::out);
		file<<text;
		file.close();
	}
}


//Express Functions
#ifdef SHRP_EXPRESS
bool Express::shrpResExp(string inPath,string outPath,bool display){
	LOGINFO("------------\nStarting Express\n");
	bool opStatus;
	//Assume that System is not mounted as default
	bool mountStatus=false;

	//To Check If the System is Mounted or not for a decision parameter which helpes us to back normal state of system mountation before Express call
	if(PartitionManager.Is_Mounted_By_Path(PartitionManager.Get_Android_Root_Path())){
		mountStatus=true;
	}else{
		mountStatus=false;
	}
	//To decide should we remount the system as RW or not
	if(!(mountStatus && minUtils::find(inPath,DataManager::GetStrValue("shrpBasePath")))){
		minUtils::remountSystem(false);
	}
	LOGINFO("Inpath - %s \nOutpath - %s \n",inPath.c_str(),outPath.c_str());
	if(TWFunc::Path_Exists(inPath)){
		LOGINFO("Inpath - Exists\n");
		if(!TWFunc::Path_Exists(outPath)){
			LOGINFO("Outpath - Not Exists\nCreating new one\n");
			TWFunc::Exec_Cmd("mkdir -p "+outPath,display,display);
		}
		if(TWFunc::Exec_Cmd("cp -r "+inPath+"* "+outPath,display,display)==0){
			LOGINFO("Executed Successfully\n");
			opStatus=true;
		}else{
			LOGINFO("Execution Failed\n");
			opStatus=false;
		}
	}else{
		LOGINFO("Inpath - Not Exists\nExiting....\n");
		opStatus=true;
	}
	//Unmounting system partition if the partition was not mounted before express call
	if(!mountStatus){
		if(PartitionManager.UnMount_By_Path(PartitionManager.Get_Android_Root_Path(),display)){
			LOGINFO("System Unmounted\n");
		}else{
			LOGINFO("System Unmount Failed \n");
		}
		unlink("/system");
		mkdir("/system", 0755);
	}
	LOGINFO("Express Processing End\n------------\n");
	return opStatus;
}
void Express::flushSHRP(){
	bool mountStatus=false;
	string basePath=DataManager::GetStrValue("shrpBasePath");

	if(PartitionManager.Is_Mounted_By_Path(PartitionManager.Get_Android_Root_Path())){
		mountStatus=true;
	}else{
		mountStatus=false;
	}
	minUtils::remountSystem(false);

	if(TWFunc::Path_Exists(basePath+"/etc/shrp")){
		TWFunc::Exec_Cmd("cp -r "+basePath+"/etc/shrp/slts /tmp/",true,true);
		TWFunc::Exec_Cmd("rm -r "+basePath+"/etc/shrp/*",true,true);
		TWFunc::Exec_Cmd("cp -r /tmp/slts "+basePath+"/etc/shrp/",true,true);
	}
	if(TWFunc::Path_Exists("/tmp/shrp")){
		TWFunc::Exec_Cmd("rm -rf /tmp/shrp",true,true);
	}
	if(!mountStatus){
		PartitionManager.UnMount_By_Path(PartitionManager.Get_Android_Root_Path(),false);
		unlink("/system");
		mkdir("/system", 0755);
	}
}

void Express::init(string basePath){
	bool mountStatus=false;
	uint64_t version=0;
	unsigned long long buildNo=1;
	DataManager::GetValue("buildNo",buildNo);
	LOGINFO("Welcome to SHRP -----------\n");
	if(PartitionManager.Is_Mounted_By_Path(basePath)){
		mountStatus=true;
	}else{
		mountStatus=false;
	}
	if(mountStatus==true){
		LOGINFO("System is already mounted\n");
		if(TWFunc::Path_Exists(basePath+"/etc/shrp/version")){
			TWFunc::read_file(basePath+"/etc/shrp/version", version);
		}
		if(version!=(uint64_t)buildNo){
			LOGINFO("Resource Version Not Matched. Mounting System as RW for further modification\n");
			minUtils::remountSystem(false);
		}
	}else{
		LOGINFO("System is not mounted\nMounting....\n");
		minUtils::remountSystem(false);
		if(TWFunc::Path_Exists(basePath+"/etc/shrp/version")){
			TWFunc::read_file(basePath+"/etc/shrp/version", version);
		}
	}

	if(version!=(uint64_t)buildNo){
		//Cloned Flush Func()
		if(TWFunc::Path_Exists(basePath+"/etc/shrp")){
			LOGINFO("Deleting Old Resources\n");
			TWFunc::Exec_Cmd("cp -r "+basePath+"/etc/shrp/slts /tmp/",true,true);
			TWFunc::Exec_Cmd("rm -r "+basePath+"/etc/shrp/*",true,true);
			TWFunc::Exec_Cmd("cp -r /tmp/slts "+basePath+"/etc/shrp/",true,true);
			TWFunc::Exec_Cmd("cp -r /twres/version "+basePath+"/etc/shrp/",true,true);
		}
		if(TWFunc::Path_Exists("/tmp/shrp")){
			TWFunc::Exec_Cmd("rm -rf /tmp/shrp",true,true);
		}
	}
	//Fetching the saved resources if available
	if(TWFunc::Path_Exists(basePath+"/etc/shrp")){
		LOGINFO("Fetching Saved Resources\n");
		TWFunc::Exec_Cmd("cp -r "+basePath+"/etc/shrp/"+"* "+"/twres/",true,true);
	}

	if(!mountStatus){
		PartitionManager.UnMount_By_Path(PartitionManager.Get_Android_Root_Path(),false);
		LOGINFO("System unmounted\n");
		unlink("/system");
		mkdir("/system", 0755);
	}
}
#endif
void Express::updateSHRPBasePath(){
	bool mountStatus=false;
	if(!PartitionManager.Is_Mounted_By_Path(PartitionManager.Get_Android_Root_Path())){
		TWFunc::Exec_Cmd("mount -w "+PartitionManager.Get_Android_Root_Path(),true);
	}else{
		mountStatus=true;
	}
	if(TWFunc::Path_Exists(PartitionManager.Get_Android_Root_Path()+"/system")){
		DataManager::SetValue("shrpBasePath",PartitionManager.Get_Android_Root_Path()+"/system");
	}else{
		DataManager::SetValue("shrpBasePath",PartitionManager.Get_Android_Root_Path());
	}
	if(!mountStatus){
		PartitionManager.UnMount_By_Path(PartitionManager.Get_Android_Root_Path(),false);
		unlink("/system");
		mkdir("/system", 0755);
	}
	LOGINFO("SHRP CURRENT BASEPATH : %s\n",DataManager::GetStrValue("shrpBasePath").c_str());
}


//Hasher Class
bool Hasher::LockPassInit(string str){
	std::ifstream f;
	arg=str;
	if(TWFunc::Path_Exists("/sdcard/SHRP/data/slts")){
		f.open("/sdcard/SHRP/data/slts", ios::in);
	}else{
		f.open("/twres/slts", ios::in);
	}
	if(!f){
		return false;
	}
	f.read(getlp,1);
	lock_pass.append(getlp,1);

	f.seekg(1,ios::beg);
	f.read(getfs,BUFFER_SIZE_SLT);
	fsalt.append(getfs,BUFFER_SIZE_SLT);

	f.seekg(BUFFER_SIZE_SLT+1,ios::beg);
	f.read(gethp,BUFFER_SIZE_PW);
	fhash.append(gethp,BUFFER_SIZE_PW);
  	f.close();
	return true;
}

bool Hasher::isPassCorrect(){
	chash = create_sha256(arg.c_str() + fsalt);
	std::string givpw=lock_pass+fsalt+chash; // reconstructed type + reconstructed salt + generated hash
	std::string recpw=lock_pass+fsalt+fhash; // reconstructed type + reconstructed salt + reconstructed hash
	return givpw==recpw ? true : false;
}

string Hasher::doHash(string str){
	string salt = create_salt(BUFFER_SIZE_SLT);
	return (salt + create_sha256(str + salt));
}


string Hasher::create_sha256(const string str)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(hash, &sha256);
    stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}

string Hasher::create_salt( size_t length ){
    static std::string chrs = "0123456789"
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    thread_local static std::mt19937 rg{std::random_device{}()};
    thread_local static std::uniform_int_distribution<std::string::size_type> pick(0, sizeof(chrs) - 2);

    std::string s;

    s.reserve(length);

    while(length--)
        s += chrs[pick(rg)];

    return s;
}



//SIG Helpers
float roundSize(float var) {
	float value = (int)(var * 100 + .5);
	return(float)value / 100;
}
void process_space(int size,int free,storageInfo storage){
	string partition,temp,tmp;
	int p_val_usage=0;
	float size_g,free_g;
	int used = size-free;
	if(size>0){
		if(free>=1024){
			free_g=(float)free/1024;
			free_g=roundSize(free_g);
			{
			stringstream buff;
			buff<<free_g;
			buff>>temp;
			tmp=temp+" GB free of ";
			}
		}else{
			{
			stringstream buff;
			buff<<free;
			buff>>temp;
			tmp=temp+" MB free of ";
			}
		}
		if(size>=1024){
			size_g=(float)size/1024;
			size_g=roundSize(size_g);
			{
			stringstream buff;
			buff<<size_g;
			buff>>temp;
			tmp=tmp+temp+" GB";
			}
		}else{
			{
			stringstream buff;
			buff<<size;
			buff>>temp;
			tmp=temp;
			tmp=tmp+temp+" MB";
			}
		}
		p_val_usage=used*100/size;
	}
	DataManager::SetValue(storage.freeStrVar,(size<=0 ? "Unavailable" : tmp.c_str()));
	DataManager::SetValue(storage.freePercentageVar,(size<=0 ? 0 : p_val_usage));
}