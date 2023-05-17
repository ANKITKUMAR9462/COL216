
#include <unordered_map>
#include <string>
#include <functional>
#include <vector>
#include <fstream>
#include <exception>
#include <iostream>
#include <boost/tokenizer.hpp>
#include <map>

// 1 cycle is equal to 1 tick

using namespace std;

int clockCycles = 0;

struct forwarding{
	int reg_1_value;
	int reg_2_value;
	bool reg1_forwarding;
	bool reg2_forwarding;
};
struct ID_stage
{
vector<string>command;
vector<string>data;
};

struct ALU_stage
{
vector<string>command;
int  value;
struct forwarding Mydata;

};

struct MEM_Stage
{
vector<string>command;
int adressvalue_dvalue;
int writedata;
}; 
 
struct WB_stage
{
	vector<string>command;
	int writedata;
};


struct MIPS_Architecture
{
	ID_stage latch2;
	ALU_stage latch3;
	MEM_Stage latch4;
	WB_stage  latch5;
    
	int registers[32] = {0}, PCcurr = 0, PCnext;	
	unordered_map<string, function<int(MIPS_Architecture &, string, string, string)>> instructions;

	int stall_register=-1;
	unordered_map<string, int> registerMap, address;
	unordered_map<int, int> memoryDelta;
	static const int MAX = (1 << 20);
	int data[MAX >> 2] = {0};
	vector<vector<string>> commands;
	vector<int> commandCount;
	enum exit_code
	{
		SUCCESS = 0,
		INVALID_REGISTER,
		INVALID_LABEL,
		INVALID_ADDRESS,
		SYNTAX_ERROR,
		MEMORY_ERROR
	};

    
	// constructor to initialise the instruction set
	MIPS_Architecture(ifstream &file)
	{
        instructions = {{"add", &MIPS_Architecture::add}, {"sub", &MIPS_Architecture::sub}, {"mul", &MIPS_Architecture::mul}, {"beq", &MIPS_Architecture::beq}, {"bne", &MIPS_Architecture::bne}, {"slt", &MIPS_Architecture::slt}, {"j", &MIPS_Architecture::j}, {"lw", &MIPS_Architecture::lw}, {"sw", &MIPS_Architecture::sw}, {"addi", &MIPS_Architecture::addi}};

		for (int i = 0; i < 32; ++i)
			registerMap["$" + to_string(i)] = i;
		registerMap["$zero"] = 0;
		registerMap["$at"] = 1;
		registerMap["$v0"] = 2;
		registerMap["$v1"] = 3;
		for (int i = 0; i < 4; ++i)
			registerMap["$a" + to_string(i)] = i + 4;
		for (int i = 0; i < 8; ++i)
			registerMap["$t" + to_string(i)] = i + 8, registerMap["$s" + to_string(i)] = i + 16;
		registerMap["$t8"] = 24;
		registerMap["$t9"] = 25;
		registerMap["$k0"] = 26;
		registerMap["$k1"] = 27;
		registerMap["$gp"] = 28;
		registerMap["$sp"] = 29;
		registerMap["$s8"] = 30;
		registerMap["$ra"] = 31;

		constructCommands(file);
		commandCount.assign(commands.size(), 0);
     
     //initilaising the memory;

    //  for(int i=0;i<100;i++)
    //  {
    //     data[i]=6;
    //  }

	}
     
	// perform add operation
	int add(string r1, string r2, string r3)
	{
 	    
        if (!checkRegisters({r1, r2, r3}) || registerMap[r1] == 0)
			return 1;
		
		 int result= registers[registerMap[r2]]+ registers[registerMap[r3]];
		// int result=latch3.pdata1+ latch3.pdata2;

		return result;
	}

	// perform subtraction operation
	int sub(string r1, string r2, string r3)
	{ 
	  int result= registers[registerMap[r2]]-registers[registerMap[r3]];
	  return result;

	}

	// perform multiplication operation
	int mul(string r1, string r2, string r3)
	{
        int result= registers[registerMap[r2]]*registers[registerMap[r3]];           
		return result;
	}


	// perform the binary operation
	int op(string r1, string r2, string r3, function<int(int, int)> operation)
	{
		if (!checkRegisters({r1, r2, r3}) || registerMap[r1] == 0)
			return 1;


		registers[registerMap[r1]] = operation(registers[registerMap[r2]], registers[registerMap[r3]]);
		PCnext = PCcurr + 1;
		return 0;
	}



	// perform the beq operation
	int beq(string r1, string r2, string label)
	{
		return bOP(r1, r2, label, [](int a, int b)
				   { return a == b; });
	}

	// perform the bne operation
	int bne(string r1, string r2, string label)
	{
		return bOP(r1, r2, label, [](int a, int b)
				   { return a != b; });
	}

	// implements beq and bne by taking the comparator
	int bOP(string r1, string r2, string label, function<bool(int, int)> comp)
	{
		if (!checkLabel(label))
			return 4;
		if (address.find(label) == address.end() || address[label] == -1)
			return 2;
		if (!checkRegisters({r1, r2}))
			return 1;
		if(comp(registers[registerMap[r1]], registers[registerMap[r2]]))
		     PCcurr=address[label];	 

		return 0;
	}

	// implements slt operation
	int slt(string r1, string r2, string r3)
	{
		if (!checkRegisters({r1, r2, r3}) || registerMap[r1] == 0)
			return 1;
         
        int result= registers[registerMap[r2]] < registers[registerMap[r3]];
        
		return result;
	}


	// perform the jump operation
	int j(string label, string unused1 = "", string unused2 = "")
	{

		if (!checkLabel(label))
			return 4;
		if (address.find(label) == address.end() || address[label] == -1)
			return 2;
		PCcurr = address[label];
		return 0;
	}

	// perform load word operation
	int lw(string r, string location, string unused1 = "")
	{
		if (!checkRegister(r) || registerMap[r] == 0)
			return 1;  
        int address;  
		address=locateAddress(location); 

		if (address < 0)
			return abs(address);
          
		return address;
	}

	// perform store word operation
	int sw(string r, string location, string unused1 = "")
	{
		if (!checkRegister(r))
			return 1;
		int address = locateAddress(location);
		if(latch3.Mydata.reg2_forwarding){
			latch3.Mydata.reg2_forwarding = false;
			My_Address(location);
		}else{
			latch4.adressvalue_dvalue = address;
		}
		return 0;
	}

	int locateAddress(string location)
	{
		if (location.back() == ')')
		{
			try
			{
				int lparen = location.find('('), offset = stoi(lparen == 0 ? "0" : location.substr(0, lparen));
				string reg = location.substr(lparen + 1);
				reg.pop_back();
				if (!checkRegister(reg))
					return -3;
				int address = registers[registerMap[reg]] + offset;

                // cout<<"SDFSFSFGFGSFSFGF  "<<address<<endl;


				if (address % 4 || address < int(4 * commands.size()) || address >= MAX)
					return -3;
				return address / 4;

			}
			catch (exception &e)
			{
				return -4;
			}
		}
		try
		{
			int address = stoi(location);
			if (address % 4 || address < int(4 * commands.size()) || address >= MAX)
				return -3;
			return address / 4;
		}
		catch (exception &e)
		{
			return -4;
		}
	}

	int My_Address(string location)
	{
		if (location.back() == ')')
		{
			try
			{
				int lparen = location.find('('), offset = stoi(lparen == 0 ? "0" : location.substr(0, lparen));
				string reg = location.substr(lparen + 1);
				reg.pop_back();
				if (!checkRegister(reg))
					return -3;
				int address = latch3.Mydata.reg_2_value + offset;
				// std::cout<<"offset "<<offset<<" "<<address<<endl;

                // cout<<"SDFSFSFGFGSFSFGF  "<<address<<endl;


				if (address % 4 || address < int(4 * commands.size()) || address >= MAX)
					return -3;
				latch4.adressvalue_dvalue = address/4;
				return address / 4;

			}
			catch (exception &e)
			{
				return -4;
			}
		}
		try
		{
			int address = stoi(location);
			if (address % 4 || address < int(4 * commands.size()) || address >= MAX)
				return -3;
			return address / 4;
		}
		catch (exception &e)
		{
			return -4;
		}
	}


	// perform add immediate operation
	int addi(string r1, string r2, string num)
	{
		if (!checkRegisters({r1, r2}) || registerMap[r1] == 0)
			return 1;

		return  registers[registerMap[r2]] + stoi(num);
	return 0;
	}


    int addi_n(int r1, int r2, string num)
	{
		return  r2 + stoi(num);
	return 0;
	}





	// checks if label is valid
	inline bool checkLabel(string str)
	{
		return str.size() > 0 && isalpha(str[0]) && all_of(++str.begin(), str.end(), [](char c)
														   { return (bool)isalnum(c); }) &&
			   instructions.find(str) == instructions.end();
	}

	// checks if the register is a valid one
	inline bool checkRegister(string r)
	{
		return registerMap.find(r) != registerMap.end();
	}

	// checks if all of the registers are valid or not
	bool checkRegisters(vector<string> regs)
	{
		return all_of(regs.begin(), regs.end(), [&](string r)
						   { return checkRegister(r); });
	}

	/*
		handle all exit codes:
		0: correct execution
		1: register provided is incorrect
		2: invalid label
		3: unaligned or invalid address
		4: syntax error
		5: commands exceed memory limit
	*/
	void handleExit(exit_code code, int cycleCount)
	{
		cout << '\n';
		switch (code)
		{
		case 1:
			cerr << "Invalid register provided or syntax error in providing register\n";
			break;
		case 2:
			cerr << "Label used not defined or defined too many times\n";
			break;
		case 3:
			cerr << "Unaligned or invalid memory address specified\n";
			break;
		case 4:
			cerr << "Syntax error encountered\n";
			break;
		case 5:
			cerr << "Memory limit exceeded\n";
			break;
		default:
			break;
		}
		if (code != 0)
		{
			cerr << "Error encountered at:\n";
			for (auto &s : commands[PCcurr])
				cerr << s << ' ';
			cerr << '\n';
		}
		cout << "\nFollowing are the non-zero data values:\n";
		for (int i = 0; i < MAX / 4; ++i)
			if (data[i] != 0)
				cout << 4 * i << '-' << 4 * i + 3 << hex << ": " << data[i] << '\n'
						  << dec;
		cout << "\nTotal number of cycles: " << cycleCount << '\n';
		cout << "Count of instructions executed:\n";
		for (int i = 0; i < (int)commands.size(); ++i)
		{
			cout << commandCount[i] << " times:\t";
			for (auto &s : commands[i])
				cout << s << ' ';
			cout << '\n';
		}
	}

	// parse the command assuming correctly formatted MIPS instruction (or label)
	void parseCommand(string line)
	{
		// strip until before the comment begins
		line = line.substr(0, line.find('#'));
		vector<string> command;
		boost::tokenizer<boost::char_separator<char>> tokens(line, boost::char_separator<char>(", \t"));
		for (auto &s : tokens)
			command.push_back(s);
		// empty line or a comment only line
		if (command.empty())
			return;
		else if (command.size() == 1)
		{
			string label = command[0].back() == ':' ? command[0].substr(0, command[0].size() - 1) : "?";
			if (address.find(label) == address.end())
				address[label] = commands.size();
			else
				address[label] = -1;
			command.clear();
		}
		else if (command[0].back() == ':')
		{
			string label = command[0].substr(0, command[0].size() - 1);
			if (address.find(label) == address.end())
				address[label] = commands.size();
			else
				address[label] = -1;
			command = vector<string>(command.begin() + 1, command.end());
		}
		else if (command[0].find(':') != string::npos)
		{
			int idx = command[0].find(':');
			string label = command[0].substr(0, idx);
			if (address.find(label) == address.end())
				address[label] = commands.size();
			else
				address[label] = -1;
			command[0] = command[0].substr(idx + 1);
		}
		else if (command[1][0] == ':')
		{
			if (address.find(command[0]) == address.end())
				address[command[0]] = commands.size();
			else
				address[command[0]] = -1;
			command[1] = command[1].substr(1);
			if (command[1] == "")
				command.erase(command.begin(), command.begin() + 2);
			else
				command.erase(command.begin(), command.begin() + 1);
		}
		if (command.empty())
			return;
		if (command.size() > 4)
			for (int i = 4; i < (int)command.size(); ++i)
				command[3] += " " + command[i];
		command.resize(4);
		commands.push_back(command);
	}

	// construct the commands vector from the input file
	void constructCommands(ifstream &file)
	{
		string line;
		while (getline(file, line))
			parseCommand(line);
		file.close();
	}


	void fivestage_piplined_bypass()
	{
		if (commands.size() >= MAX / 4)
		{
			handleExit(MEMORY_ERROR, 0);
			return;
		}

		int clockcycle=0;

		vector<int>commandlist;
		vector<vector<string>>storedcommands;
		PCcurr=0;
		int stall_count=0;
		bool stall_initiated=false;
		bool branch_stall=false;
	
	while(1)
	{
		 
	    printRegistersAndMemoryDelta(clockcycle);
		++clockcycle;


	//******************************************WB_STAGE***************************************************
	if(!latch5.command.empty())
	 {	
		if(latch5.command[0]=="add" || latch5.command[0]=="sub" || latch5.command[0]=="mul" || latch5.command[0]=="slt" || latch5.command[0]=="addi" || latch5.command[0]=="lw")
		{
			registers[registerMap[latch5.command[1]]]=latch5.writedata;
		}

		latch5.command.clear();
		// latch5.writedata=0;
		storedcommands.erase(storedcommands.begin());

	 }

	//***************************************MEM_STAGE**************************************************
	if(!latch4.command.empty())
	{ 
	   latch5.command=latch4.command;
	   
	   if(latch4.command[0]=="add" || latch4.command[0]=="sub" || latch4.command[0]=="mul" || latch4.command[0]=="slt" || latch4.command[0]=="addi")
		{
		 latch5.writedata=latch4.adressvalue_dvalue;
		}

		if(latch4.command[0]=="sw")
		{
		  data[latch4.adressvalue_dvalue]=latch4.writedata;
		  memoryDelta[latch4.adressvalue_dvalue]=latch4.writedata;
		//  std::cout<<latch4.adressvalue_dvalue<<" sffsfdf"<<endl;
		}

		if(latch4.command[0]=="lw")
		{
		  latch5.writedata=data[latch4.adressvalue_dvalue];
		}
	  
	  latch4.command.clear();
	//latch4.adressvalue_dvalue=0;
	}

	//*******************************ALU STAGE**************************************************
	if(!latch3.command.empty())
  {
		// cout<<"clock :: "<<clockcycle<<latch3.command[0]<<endl;
		latch4.command=latch3.command;		
		
		if(latch3.command[0]=="sw")
			{

				sw(latch3.command[1],latch3.command[2],latch3.command[3]);
		
				if(latch3.Mydata.reg1_forwarding){
					latch4.writedata = latch3.Mydata.reg_1_value;
					latch3.Mydata.reg1_forwarding= false;
				}
				else{
					latch4.writedata = registers[registerMap[latch3.command[1]]];
				}
			}

		if(latch3.command[0]=="lw")
		{	
			if(latch3.Mydata.reg2_forwarding){
				My_Address(latch3.command[2]);
				latch3.Mydata.reg2_forwarding = false;
			}
			else{
			int adr=lw(latch3.command[1],latch3.command[2],latch3.command[3]);
			latch4.adressvalue_dvalue=adr;
			}
		}


		if(latch3.command[0]=="beq" )
		{

			if(latch3.Mydata.reg1_forwarding == true && latch3.Mydata.reg2_forwarding == true){
				latch3.Mydata.reg1_forwarding == false; 
				latch3.Mydata.reg2_forwarding == false;
				if(latch3.Mydata.reg_1_value == latch3.Mydata.reg_2_value){
					PCcurr = address[latch3.command[3]];
				}
				
			}
			else if( latch3.Mydata.reg1_forwarding == true){
				if(latch3.Mydata.reg_1_value == registers[registerMap[latch3.command[2]]]){
					PCcurr = address[latch3.command[3]];
				}
				latch3.Mydata.reg1_forwarding == false; 	
			}
			else if (latch3.Mydata.reg2_forwarding == true)
			{
				if(registers[registerMap[latch3.command[1]]] == latch3.Mydata.reg_2_value){
					PCcurr = address[latch3.command[3]];

				}
				latch3.Mydata.reg2_forwarding == false;		
			}
			else{
			if(registers[registerMap[latch3.command[2]]] == registers[registerMap[latch3.command[1]]])
					PCcurr = address[latch3.command[3]];

			}
			branch_stall=false;
			latch3.command.clear();
			latch3.value=0;
			 continue;

		}
		if(latch3.command[0]=="bne"){
			if(latch3.Mydata.reg1_forwarding == true && latch3.Mydata.reg2_forwarding == true){
				latch3.Mydata.reg1_forwarding == false; 
				latch3.Mydata.reg2_forwarding == false;
				if(latch3.Mydata.reg_1_value != latch3.Mydata.reg_2_value){
					PCcurr = address[latch3.command[3]];
				}
				
			}
			else if( latch3.Mydata.reg1_forwarding == true){
				if(latch3.Mydata.reg_1_value != registers[registerMap[latch3.command[2]]]){
					PCcurr = address[latch3.command[3]];
				}
				latch3.Mydata.reg1_forwarding == false; 	
			}
			else if (latch3.Mydata.reg2_forwarding == true)
			{
				if(registers[registerMap[latch3.command[1]]] != latch3.Mydata.reg_2_value){
					PCcurr = address[latch3.command[3]];

				}
				latch3.Mydata.reg2_forwarding == false;		
			}
			else{
			if(registers[registerMap[latch3.command[1]]] != registers[registerMap[latch3.command[2]]])
					PCcurr = address[latch3.command[3]];

			}
			branch_stall=false;
			latch3.command.clear();
			latch3.value=0;
			continue;
		}
		if(latch3.command[0]=="add")
		 {
			if(latch3.Mydata.reg1_forwarding == true && latch3.Mydata.reg2_forwarding == true){
				latch4.adressvalue_dvalue = latch3.Mydata.reg_1_value + latch3.Mydata.reg_2_value;
				latch3.Mydata.reg1_forwarding == false; 
				latch3.Mydata.reg2_forwarding == false;
			}
			else if( latch3.Mydata.reg1_forwarding == true){
				latch4.adressvalue_dvalue = latch3.Mydata.reg_1_value + registers[registerMap[latch3.command[3]]];
				latch3.Mydata.reg1_forwarding == false; 	
			}
			else if (latch3.Mydata.reg2_forwarding == true)
			{
				latch4.adressvalue_dvalue =  registers[registerMap[latch3.command[2]]] + latch3.Mydata.reg_2_value;
				latch3.Mydata.reg2_forwarding == false;	
				// cout<<"latch 4 value after the add function:: "<<latch4.adressvalue_dvalue<<endl;	
			}
			else{
			latch4.adressvalue_dvalue =  registers[registerMap[latch3.command[2]]] + registers[registerMap[latch3.command[3]]];

			}
			
			
		 }
		 
		 if(latch3.command[0]=="sub")
		 {
			if(latch3.Mydata.reg1_forwarding == true && latch3.Mydata.reg2_forwarding == true){
				latch4.adressvalue_dvalue = latch3.Mydata.reg_1_value - latch3.Mydata.reg_2_value;
				latch3.Mydata.reg1_forwarding == false; 
				latch3.Mydata.reg2_forwarding == false;
			}
			
			else if( latch3.Mydata.reg1_forwarding == true){
				latch4.adressvalue_dvalue = latch3.Mydata.reg_1_value - registers[registerMap[latch3.command[3]]];
				latch3.Mydata.reg1_forwarding == false; 	
			}
			else if (latch3.Mydata.reg2_forwarding == true)
			{
				latch4.adressvalue_dvalue =  registers[registerMap[latch3.command[2]]] - latch3.Mydata.reg_2_value;
				latch3.Mydata.reg2_forwarding == false;		
			}
			else{
			latch4.adressvalue_dvalue =  registers[registerMap[latch3.command[2]]] - registers[registerMap[latch3.command[3]]];

			}
			
		 }
		 
		 if(latch3.command[0]=="mul")
		 {
			if(latch3.Mydata.reg1_forwarding == true && latch3.Mydata.reg2_forwarding == true){
				latch4.adressvalue_dvalue = latch3.Mydata.reg_1_value * latch3.Mydata.reg_2_value;
				latch3.Mydata.reg1_forwarding == false; 
				latch3.Mydata.reg2_forwarding == false;
			}
			
			else if( latch3.Mydata.reg1_forwarding == true){
				latch4.adressvalue_dvalue = latch3.Mydata.reg_1_value * registers[registerMap[latch3.command[3]]];
				latch3.Mydata.reg1_forwarding == false; 	
			}
			else if (latch3.Mydata.reg2_forwarding == true)
			{
				latch4.adressvalue_dvalue =  registers[registerMap[latch3.command[2]]] * latch3.Mydata.reg_2_value;
				latch3.Mydata.reg2_forwarding == false;		
			}
			else{
			latch4.adressvalue_dvalue =  registers[registerMap[latch3.command[2]]] * registers[registerMap[latch3.command[3]]];

			}
			
		 }
		 
		 if(latch3.command[0]=="slt")
		 {
			if(latch3.Mydata.reg1_forwarding == true && latch3.Mydata.reg2_forwarding == true){
				latch4.adressvalue_dvalue = latch3.Mydata.reg_1_value < latch3.Mydata.reg_2_value;
				latch3.Mydata.reg1_forwarding == false; 
				latch3.Mydata.reg2_forwarding == false;
			}
			else if( latch3.Mydata.reg1_forwarding == true){
				latch4.adressvalue_dvalue = latch3.Mydata.reg_1_value < registers[registerMap[latch3.command[3]]];
				latch3.Mydata.reg1_forwarding == false; 	
			}
			else if (latch3.Mydata.reg2_forwarding == true)
			{
				latch4.adressvalue_dvalue =  registers[registerMap[latch3.command[2]]] < latch3.Mydata.reg_2_value;
				latch3.Mydata.reg2_forwarding == false;		
			}
			
			else{
			latch4.adressvalue_dvalue =  registers[registerMap[latch3.command[2]]] < registers[registerMap[latch3.command[3]]];

			}
			
		 
		 }
		 
		 if(latch3.command[0]=="addi")
		 {  
			if(latch3.Mydata.reg1_forwarding){
				latch3.Mydata.reg1_forwarding == false; 
				latch4.adressvalue_dvalue = latch3.Mydata.reg_1_value +stoi(latch3.command[3]);
			}else{
				latch4.adressvalue_dvalue = registers[registerMap[latch3.command[2]]] +stoi(latch3.command[3]);

			}
		 }

	latch3.command.clear();
	latch3.value=0;

	}


	//*****************************DECODE STAGE***************************************************
	if(!latch2.command.empty())
	{
	  if(latch2.command[0]=="j")
	  {
		instructions[latch2.command[0]](*this, latch2.command[1], latch2.command[2], latch2.command[3]);
		branch_stall=false;
		latch2.command.clear();
		storedcommands.pop_back();
		continue;
	  }
	  
	  
	  if(stall_count!=0)
	          stall_count--;
	  
	   
	  if(stall_initiated==false)	
	  {
		if(latch2.command[0]=="add" || latch2.command[0]=="sub" || latch2.command[0]=="mul" || latch2.command[0]=="slt" || latch2.command[0]=="addi" || latch2.command[0]=="lw" || latch2.command[0]=="sw" || latch2.command[0]=="beq" || latch2.command[0]=="bne")
		{
			string consumer1=latch2.command[2];
			string consumer2=latch2.command[3];
			
			if(latch2.command[0]=="lw" || latch2.command[0]=="sw")
			 {
				if(latch2.command[2].back() == ')')
			     { 
					int lparen = latch2.command[2].find('('), offset = stoi(lparen == 0 ? "0" : latch2.command[2].substr(0, lparen));
				    consumer2 = latch2.command[2].substr(lparen + 1);
				    consumer2.pop_back();
					// std::cout<<consumer2<<" consumer 2 "<<std::endl;
				 }
				
				else
				consumer2="M";  
			
			 }
				//cout<<"consumer 2 is :: "<<consumer2<<endl;
			 if(latch2.command[0]=="sw")
			  {
				consumer1=latch2.command[1];
			  }

			 if(latch2.command[0]=="lw")
			  {
				consumer1="M";
			  }

			 if(latch2.command[0]=="beq")
			  {
				consumer2=latch2.command[2];
				consumer1=latch2.command[1];
			  }

			 if(latch2.command[0]=="bne")
			  {consumer2=latch2.command[2];
				consumer1=latch2.command[1];
			  }
						
			 string producer;
	


		 if(!latch4.command.empty())
		 {
			if(latch4.command[0]=="add" || latch4.command[0]=="sub" || latch4.command[0]=="mul" || latch4.command[0]=="slt" || latch4.command[0]=="addi" || latch4.command[0]=="lw" )
			    producer=latch4.command[1];

			if(producer==consumer1)
			   {
				latch3.Mydata.reg1_forwarding =true;
				latch3.Mydata.reg_1_value = latch4.adressvalue_dvalue;

				if(latch4.command[0]=="lw" && latch2.command[0]!="sw")
				   {
					// latch3.Mydata.reg_1_value = latch5.writedata;
					stall_register=1;
				     stall_count=1;
					 stall_initiated=true;
					 continue;
				   }

				 else if(latch4.command[0]=="lw" && latch2.command[0]=="sw")
				   {
					latch3.Mydata.reg_1_value = latch5.writedata;
				   }


		   }else{
				latch3.Mydata.reg1_forwarding = false;
			   }
			   
			if(producer==consumer2)
			   {
				
				latch3.Mydata.reg2_forwarding =true;
				latch3.Mydata.reg_2_value = latch4.adressvalue_dvalue;				

				if(latch4.command[0]=="lw")
				   { stall_register=2;
				     stall_count=1;
					 stall_initiated=true;
					 continue;
				   }

		   }else{
				latch3.Mydata.reg2_forwarding = false;
			   }


		 }
		
		
		
		}
	 }	
		
		if(stall_count==0 && stall_initiated==true && stall_register==2)
		        latch3.Mydata.reg_2_value = latch5.writedata;

		if(stall_count==0 && stall_initiated==true && stall_register==1)
		        latch3.Mydata.reg_1_value = latch5.writedata;		


		//transfering the command value from it
		   if(stall_count==0)	
			{ 
				latch3.command=latch2.command;
			    stall_initiated=false;
				stall_register=-1;
	            latch2.command.clear();
			} 

	}

	//**********************IF STAGE***********************************************************
	 if(PCcurr>=commands.size())  //encountered all commands;
	 {
		if(storedcommands.empty()==true)
		{ 
			printRegistersAndMemoryDelta(clockcycle);
		    break; //program completed
		}
		
		continue;
	
	  }

	
	
	 vector<string>command=commands[PCcurr];
	
	
	
	 if(stall_count==0 && branch_stall==false)
	    {  storedcommands.push_back(command);
			latch2.command=command;
			  
			  if(command[0]=="beq" ||command[0]=="bne" || command[0]=="j")
			     branch_stall=true;
	           PCcurr++;
		}


    }
	// handleExit(SUCCESS,clockcycle);

}
       
    // execute the commands sequentially (no pipelining)
	void executeCommandsUnpipelined()
	{
	
		if (commands.size() >= MAX / 4)
		{
			handleExit(MEMORY_ERROR, 0);
			return;
		}

		int clockCycles = 0;
		while (PCcurr < commands.size())
		{
			++clockCycles;
			vector<string> &command = commands[PCcurr];
			if (instructions.find(command[0]) == instructions.end())
			{
				handleExit(SYNTAX_ERROR, clockCycles);
				return;
			}
			exit_code ret = (exit_code) instructions[command[0]](*this, command[1], command[2], command[3]);
			if (ret != SUCCESS)
			{
				handleExit(ret, clockCycles);
				return;
			}
			++commandCount[PCcurr];
			PCcurr = PCnext;
			printRegisters(clockCycles);
		}
		handleExit(SUCCESS, clockCycles);
	}


	
	// print the register data in hexadecimal
	void printRegisters(int clockCycle)
	{
		cout << "Cycle number: " << clockCycle <<endl;	
		for (int i = 0; i < 32; ++i)
			cout << registers[i] << ' ';

		cout<<endl;	
	}

	void printRegistersAndMemoryDelta(int clockCycle)
	 {
		for (int i = 0; i < 32; ++i)
			cout << registers[i] << ' ';
		    cout << '\n';
		    cout << memoryDelta.size() << ' ';
		for (auto &p : memoryDelta)
			cout << p.first << ' ' << p.second;
		memoryDelta.clear();
		cout<<endl;

	 }

};

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		std::cerr << "Required argument: file_name\n./MIPS_interpreter <file name>\n";
		return 0;
	}
	std::ifstream file(argv[1]);
	MIPS_Architecture *mips;
	if (file.is_open())
		mips = new MIPS_Architecture(file);
	else
	{
		std::cerr << "File could not be opened. Terminating...\n";
		return 0;
	}
   
	 mips->fivestage_piplined_bypass();
	return 0;
}