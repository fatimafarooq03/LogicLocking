//
//  main.cpp
//  RLL
//
//  Created by Fatima Farooq on 19/06/2023.
//

#include <iostream>
#include <string>
#include <cmath>
#include <fstream>
#include <sstream>
#include <vector>
#include <regex>
#include <cstdlib>
#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <list>
#include <cmath>


using namespace std;

bool isSimilar(int array[], int size, int element);
void createVectors(string inputFile, int n);

int main() {
    
    srand(time(NULL));
    
    string filename; // unlocked input circuit
    
    cout << "Enter orignal circuit file name" << endl;
    cin >> filename;
    
    // to synthesize the unlocked circuit
    string abcCommand = "./abc -c 'read_lib 45nm.lib; read -m " + filename + "; strash; refactor; map; write synthesized_" + filename + "'";
    system(abcCommand.c_str());

    // to keep track of size of key
    int keySize;
    
    // ask the user for the size of key
    cout << "Enter size of key" << endl;
    cin >> keySize;
    
    int maxKeySize = keySize;
    
    keySize = ceil((double)keySize/2); // selecting half of gates to lock
    
    // creating key
    vector<int> key(keySize);
    
    for (int i = 0; i < keySize; i++)
    {
        key[i]= (rand()%2);
    }
    
    // printing the key:
    cout << "The secret key is: " << endl;
    for (int i = 0; i < keySize; i++)
    {
        cout << key[i];
    }
    cout << endl;
    
    // opening the input file (unlocked circuit)
    ifstream infile;
    infile.open(filename);
    
    if (infile.fail())
    {
        cerr << "Error opening input file" << endl;
        exit(-1);
    }
    
    
    // opening the output file which would have the locked circuit
    ofstream outfile;
    outfile.open("locked_" + filename);
    
    if (outfile.fail())
    {
        cerr << "Error opening output file" << endl;
        exit(-1);
    }
    
    // printing the key to the output file
    outfile << "//Secret key is: ";
    for (int i = 0; i < keySize; i++)
    {
        outfile << key[i];
    }
    outfile << endl;
    
    vector<string> gates; // vector to store all the gates in the circuit
    
    
    string line; // variable to read in from the file
    
    
    regex gate("U\\d+\\s*\\("); // regex expression that matches to a gate U followed by digit and/ot non-whitespace and opening bracket
    
    // flag type for determining the matching behavior
       // here it is for matches on 'string' objects
       smatch m;
    
    string gate2; // variable to store the matched gate
    
    // storing gates
    while(!infile.eof())
    {
        getline(infile,line); // read from the file line by line
        if (regex_search(line, m, gate)) // searches for a gate in each line
        {
            gate2 = m[0].str();
            gate2 = regex_replace(gate2,regex("\\s*\\("),"");
            gates.push_back(gate2); // addes the gate name to the gates vector
        }
    }
    
    long gateNum = 0; // variable to track the number of gates
    
    gateNum = gates.size();
    
    // creating array for gate indexes selected
    int* RLLgates = new int [keySize];
    

        
    // selecting random gates to lock
    int num; // variable to randomly select an index
    for (int i = 0; i < keySize; i++)
    {
        do
        {
            num = rand()%(gateNum);
        }
        
        while(isSimilar(RLLgates, i, num) && !(regex_match(gates[num],gate)));
        RLLgates[i]=num;
    }
    
   string sectionEnd("\\)?\\;"); // regex expression to check if an end of section has beeen reached
    int sectionCounter = 0; // counter to keep track of which section you are at
    
    // creating key input string to be added to the module section
    string keyInput = "";
    for (int i = 0; i < keySize; i++)
    {
        keyInput = keyInput + ", " + "KEYINPUT" + to_string(i);
    }
    
    keyInput = keyInput + ");";
    
    
    // creating second input string to add to the input section
    string keyInput2 = "";
    for (int i = 0; i < keySize; i++)
    {
        keyInput2 = keyInput2 + "input KEYINPUT" + to_string(i) + ";\n";
    }
    
    keyInput2 = keyInput2 + "//Header:";
    
    // creating key wire string to be added to the wires section
    string keyWires = "";
    for (int i = 0; i < keySize; i++)
    {
        keyWires = keyWires + ";\n" + "wire RLL_wire_A_" + to_string(i);
    }
    
    keyWires = keyWires + ";";
    
    
    regex output("\\.(ZN|Z)\\(\\w+\\)"); // regex expression to check for output that starts with ZN/Z followed by non-whitespace character
    
    regex wire("\\(n\\d+\\)"); // regex expression to check for wire that starts with (n and is follwed by digitis and ends with )
    
    regex outputStart("\\.(ZN|Z)");

    
    int zeros(0),ones(0); // to keep track of the number of zeros and ones in the key
    
    // so that the file cursor returns to the beginning of the file
    infile.close();
    infile.open(filename);
    
    smatch s,y,t,k;   // flag type for determining the matching behavior
    
    string line2; // string variable to store another line read from the file while conserving line
    
    string OutputWire; // variable to store the output wire of the selected gate to lock
    
    
    int lineCounter = 0; // to keep track of the number of lines
    
    regex lineEnd("\\.(Z|ZN)\\(\\w+\\)\\s*\\)\\s*\\;");
    
    while(!infile.eof())
    {
        getline(infile,line);
        lineCounter++;
    
        // to skip a line
        
        if (regex_search(line,k,regex("U\\d+\\s*\\(")))
        {
            if (!(regex_search(line,y,lineEnd)))
            {
                getline(infile,line2);
                line2 = regex_replace(line2,regex("^\\s+"),"");
                line = regex_replace(line,regex("\n"),"");
                line = line + line2;
            }
        }
        
        if (lineCounter == 6)
        {
            outfile << "//Module:" << endl;
        }
        
        if  (regex_search(line, s, regex(sectionEnd)))  // searches for the end of section in the line
        {
            sectionCounter++;
            
            if (sectionCounter == 1) // this recognizes the end of module section and then adds the input key string to the module section
            {
                line = regex_replace(line, regex(sectionEnd), "");
                line = line + keyInput + "\n" + keyInput2;
               // regex_replace(back_inserter(line), line.begin(), line.end(),
                                  //sectionEnd,  keyInput);
    
            }
            else if (sectionCounter == 4) // this recognizes the end of wires section and then adds the key wires string to the wires section

            {
               line = regex_replace(line, regex(sectionEnd), "");
                line = line + keyWires + "\n//Body:";

             
            }
        }

        
        // loop to go through all the n key inputs and n gates
            
            for (int i = 0; i < keySize; i++)
            {

                gate = gates[RLLgates[i]];
                
                // this assigns the selected RLL gate to the gate regex
                //regex_search(line, m, gate);
                if (regex_search(line, t, regex(gate))) // searches for that gate in each line
                {
                    
                    regex_search(line, t, output); // if the gate is found, searches for the output gate in the line
                    OutputWire = t[0]; // assigns the output gate to the output wire
                    
                    //replaces the gate in OutputWire to just the output wire
   
            
                    // replaces the output gate with RLL wire
                    

                    if (regex_search(OutputWire,s,regex("\\.Z\\(")))
                    {
                        
                        
                        line = regex_replace(line,output,".Z(RLL_wire_A_" + to_string(i) + ")");
                    }
                    
                    else
                    {
                        line = regex_replace(line,output,".ZN(RLL_wire_A_" + to_string(i) + ")");
                    }
                    
                    
                    // if the key input is 0, assigns XOR gate and adds the previous output wire to the output of this XOR gate
                    if (key[i] == 0)
                    {
                        //replaces the gate in OutputWire to just the output wire
                        OutputWire = regex_replace(OutputWire,outputStart,".Z");
                        line = line + "\n" +"XOR2_X1 RLL_XOR_" + to_string(zeros) + " (.A(RLL_wire_A_" + to_string(i) + "), .B(KEYINPUT" + to_string(i) + ")," + OutputWire + ");";
                        zeros++;
        
                    }
                    
                    // if the key input is 1, assigns XNOR gate and adds the previous output wire to the output of this XNOR gate
                    else if (key[i] == 1)
                        
                    {
                        //replaces the gate in OutputWire to just the output wire
                        OutputWire = regex_replace(OutputWire,outputStart,".ZN");
                        line = line + "\n" +"XNOR2_X1 RLL_XNOR_" + to_string(ones) + " (.A(RLL_wire_A_" + to_string(i) + "), .B(KEYINPUT" + to_string(i) + ")," + OutputWire + ");";
                        ones++;
                    }
                    
                    break;
                }
            
            }
        
        if (lineCounter > 5)
        {
            outfile << line << endl;
        }
    }
    
    cout << "The circuit" << filename << " is successfully locked." << endl; // prints a confirmation message to the console
    
    // encoding of the locked circuit
    createVectors("locked_"+filename,keySize);
    
    // synthesizing the locked circuit
    
 abcCommand = "./abc -c 'read_lib 45nm.lib; read -m locked_" + filename + "; strash; refactor; map; write synthesized_locked_" + filename + "'";
    system(abcCommand.c_str());
    
    createVectors("synthesized_locked_" + filename,keySize);

    delete [] RLLgates;
   
    outfile.close();
    
    
// comparing the encoding before and after synthesis of locked circuit
    
    ifstream infile2; // another ifstream to open locked vectors
    infile2.open("Vectors_locked_"+filename);
    
    
    outfile.open("Vectors_compared_"+filename); // outputting the comparison to the output file

    
    // vectors to store each vector from both the vector files
    vector <string> comparedVec(6);
    vector <string> comparedVec2(6);

    
    infile.close();
    infile.open("Vectors_synthesized_locked_"+filename); // opening the syntheszied locked vectors
    while(!infile.eof())
    {
        for (int i = 0; i < 6; i++)
        {
            infile >> comparedVec[i]; // storing each synthesized vector to compare
        }
            
            // to always reset the read cursor to he beginning
            infile2.close();
            infile2.open("Vectors_locked_"+filename);
            
            while(!infile2.eof()) // goes through all the pre-synthesized vectors for each synthesized vector
            {
                for (int j = 0; j < 6; j++)
                {
                    infile2 >> comparedVec2[j]; //storing each pre-synthesized vector to compare
                }
                
                // for the same key input, checks if the keygate changed from XNOR to XOR or vice versa
                if (comparedVec[0]==comparedVec2[0])
                {
                    if ((comparedVec2[4]=="XOR" && comparedVec[4]=="XNOR") || (comparedVec2[4]=="XNOR" && comparedVec[4]=="XOR"))
                    {
                        outfile << comparedVec[0] + "," << endl;
                        break; // exit thr loop if match for that specific key vector from the synthesized vectors are found
                    }
                }
            }
        }
    
    // closing all the opened files
    outfile.close();
    infile.close();
    infile2.close();
   
   // generating 1s and 0s output comparison file
outfile.open("boolean_Vectors_compared_"+filename);
   
   for (int i  =  0; i < keySize; i++)
   {
      int isChanged = 0; // variable to track if the keygate changed
      infile.open("Vectors_compared_"+filename);
      while(!infile.eof())
      {
         getline(infile,line);
         if (regex_search(line,m,regex("KI"+to_string(i)+"\,")))
         {
            isChanged = 1;
            break;
         }
      }
      outfile << isChanged << endl;
      infile.close(); // to reset the file pointer to the beginning of the file
   }
   
  // closing all files opened
   infile.close();
   outfile.close();
   
   
    // locking similar to the changed gates
    
   
   ifstream infile1; // another ifstream to open syntheszied locked vectors

    string line3,line4; // additional string variables to read from files
    
    string wire2, gateType; // additional string variables to find output wires and gate type of gates
    ifstream infile3,infile4; // additonal ifstreams to read from more files at the same time
    
    const int VectorSize = 6; // defining the vector size to 6
    vector<string>  synthVector(VectorSize); //vector to store each synthesized vector from the synthesized vectors file
    
    // vectors to store the matched gates that would be locked using XOR ot XNOR
    vector <string> XORgates;
    vector <string> XNORgates;
    

    bool matchFound = false; // variable to check if the gate matches a changed vector
   int totalMatchGates = 0; // variable to track the number of matched gates found
    
    infile.open("synthesized_locked_"+filename); // opens the synthesized circuit
    while(getline(infile,line))
        {
           totalMatchGates = XNORgates.size() + XORgates.size();
            matchFound = false;
            if ( totalMatchGates >= floor(double(maxKeySize/2)))
            {
                break; // if adequate gates to lock to complete the key are found, exit the loop
            }

            streampos currentPosition = infile.tellg(); // remembers the current position in infile
            infile.close(); // closes the file so the same file can be read again in the upcoming loops
            if (regex_search(line,m,regex("(U|g)\\d+\\s*\\("))) // searches if the line contains a gate
            {
              infile2.close();
               infile2.open("Vectors_synthesized_locked_"+filename);
               while(!infile2.eof())
               {
                  for (int j = 0; j < VectorSize; j++)
                  {
                     infile2 >> synthVector[j]; // stores each vector from the synthesized vectors file
                     
                  }
                  
                  // opens and resets the lcoked vectors file to the beginning
                  infile4.close();
                  infile4.open("Vectors_locked_"+filename);
                  while(!infile4.eof()) // goes through all the pre-synthesized vectors for each synthesized vector
                  {
                     for (int j = 0; j < VectorSize; j++)
                     {
                        infile4 >> comparedVec2[j]; //storing each pre-synthesized vector to compare
                        
                     }
                     // for the same key input, checks if the keygate changed from XNOR to XOR or vice versa
                     if (synthVector[0]==comparedVec2[0])
                     {
                        if ((comparedVec2[4]=="XOR" && synthVector[4]=="XNOR") || (comparedVec2[4]=="XNOR" && synthVector[4]=="XOR"))
                        {
                           regex_search(line,s,regex("\\w+X\\d")); // finds the gate type in the line
                           gateType = s[0].str();
                           gateType = regex_replace(gateType,regex("\\d*_X\\d"),""); // gets just the name of the gate type
                           
                           // compares if the gate type is the same as the gate input of the changed keygate
                           if (gateType == synthVector[1])
                           {
                              regex_search(line,t,regex("\\.(ZN|Z)\\(\\w+\\)"));
                              wire2 = t[0].str();
                              wire2 = regex_replace(wire2,regex("\\.(ZN|Z)"),"");
                              wire2 = regex_replace(wire2,regex("\\(|\\)"),"");
                              
                              // finds the output of the gate if previous condition met
                              
                              // opens and resets the synthesized lcoked vectors file to the beginning
                              infile3.close();
                              infile3.open("synthesized_locked_"+filename);
                              while (!infile3.eof())
                              {
                                 getline(infile3,line4);
                                 if (regex_search(line4,k,regex ("\\.\\w+\\(" + wire2 + "\\)")))
                                 {
                                    string gate5 = k[0].str();
                                    if (!regex_search(gate5,s,regex("\\.(ZN|Z)\\(")))
                                    {
                                       regex_search(line4,k,regex("\\w+X\\d"));
                                       gateType = k[0].str();
                                       gateType =regex_replace(gateType,regex("\\d*_X\\d"),"");
                                       
                                       // checks if gate type is the same as the output gate of the changed key gate
                                       if (gateType == synthVector[5])
                                       {
                                          // checks if the changed keygate is XNOR or XOR and adds the matched gate to the corresponding XNOR/XOR gates vector
                                          if (synthVector[4]=="XOR")
                                          {
                                             matchFound = true;
                                             XORgates.push_back(m[0].str());
                                             break; // exits the synthesized locked circuit file as if match of the read gate to a changed vector is found
                                          }
                                          else if (synthVector[4]=="XNOR")
                                          {
                                             matchFound = true;
                                             XNORgates.push_back(m[0].str());
                                             break;
                                          }
                                          
                                       }
                                    }
                                    
                                 }
                                 
                              }
                              
                              // exists the locked vectors circuit file as if match of the read gate to a changed vector is found
                              if (matchFound== true)
                              {
                                 break;
                              }
                           }
                        }
                     }
                     // exists the synthesized locked vectors circuit file as if match of the read gate to a changed vector is found
                  }
                  if (matchFound == true)
                  {
                     break;
                     
                  }
               }
            }
           infile.open("synthesized_locked_"+filename);
           infile.seekg(currentPosition);
        }
   infile.close();
   
   totalMatchGates += keySize;
   
   maxKeySize = min(totalMatchGates,maxKeySize); // reassigns the maximum key size to the actual one made by the code
   
   
   // creating key input string to be added to the module section
keyInput = "";
   for (int i = keySize; i < maxKeySize; i++)
   {
       keyInput = keyInput + "KEYINPUT" + to_string(i) + ", ";
   }
   
   
   // creating second input string to add to the input section
   keyInput2 = "input ";
   for (int i = keySize; i < maxKeySize; i++)
   {
       keyInput2 = keyInput2 + "KEYINPUT" + to_string(i) + ", ";
   }
   
   // creating key wire string to be added to the wires section
   keyWires = "wire ";
   for (int i = keySize; i < maxKeySize; i++)
   {
       keyWires = keyWires + "RLL_wire_A_" + to_string(i) + ", ";
   }
   

   
   // opens the final synthesized output file that will have the final locking
   outfile.open("final_synthesized_locked"+filename);
    
   
   // generating remaining key inputs and locking the cirucit using them

   infile.open("synthesized_locked_"+filename); // opening the synthesized locked circuit
   
   int i  = keySize; // to track the key input

   while(!infile.eof())
   {
      getline(infile,line); // reads a line from the file

      
      // adding the key inputs to the module section
      if (regex_search(line,m,regex("module\\s")))
      {
         line = line + "\n" + keyInput;
      }
   
      // adding the key inputs to the input section
      if (regex_search(line,m,regex("input")))
      {
         line = regex_replace(line,regex("input"),keyInput2);
      }
      
      // adding the wires to the wires section
      if (regex_search(line,m,regex("wire")))
      {
         line = regex_replace(line,regex("wire"),keyWires);
      }
            
            if( i < maxKeySize)
            {
                // checks if the line contains any of the matched gates that would be locked using XOR
            for (int j = 0; j < XORgates.size(); j++)
            {
                XORgates[j]=regex_replace(XORgates[j],regex("\\(|\\)"),"");
                if(regex_search(line,m,regex(XORgates[j])))
                {
                    regex_search(line, t, output); // if the gate is found, searches for the output gate in the line
                    OutputWire = t[0].str(); // assigns the output gate to the output wire

                    
                   // replaces the output wire to the RLL wire
                    if (regex_search(OutputWire,s,regex("\\.Z\\(")))
                    {
                        line = regex_replace(line,output,".Z(RLL_wire_A_" + to_string(i) + ")");
                    }
                    
                    else
                    {
                        line = regex_replace(line,output,".ZN(RLL_wire_A_" + to_string(i) + ")");
                    }
                    
                    //replaces the gate in OutputWire to just the output wire
                    OutputWire = regex_replace(OutputWire,outputStart,".Z");
                    line = line + "\n" +"XOR2_X1 RLL_XOR_" + to_string(zeros) + " (.A(RLL_wire_A_" + to_string(i) + "), .B(KEYINPUT" + to_string(i) + ")," + OutputWire + ");";
                    zeros++;
                    key.push_back(0);
                    i++;
                   // gateFound = true;
                    break;
                }
            }
            
               // checks if the line contains any of the matched gates that would be locked using XNOR
            for (int j = 0; j < XNORgates.size(); j++)
            {
           
                XNORgates[j]=regex_replace(XNORgates[j],regex("\\(|\\)"),"");
                if(regex_search(line,m,regex(XNORgates[j])))
                {
                    regex_search(line, t, output); // if the gate is found, searches for the output gate in the line
                    OutputWire = t[0].str(); // assigns the output gate to the output wire
   
                    // replaces the output wire to the RLL wire
                    if (regex_search(OutputWire,s,regex("\\.Z\\(")))
                    {
                        line = regex_replace(line,output,".Z(RLL_wire_A_" + to_string(i) + ")");
                    }
                    
                    else
                    {
                        line = regex_replace(line,output,".ZN(RLL_wire_A_" + to_string(i) + ")");
                    }
                    
                    //replaces the gate in OutputWire to just the output wire
                    OutputWire = regex_replace(OutputWire,outputStart,".ZN");
                    line = line + "\n" +"XNOR2_X1 RLL_XNOR_" + to_string(ones) + " (.A(RLL_wire_A_" + to_string(i) + "), .B(KEYINPUT" + to_string(i) + ")," + OutputWire + ");";
                    ones++;
                    key.push_back(1);
                    i++;
                    break; 
                }
            }
    
        }
            outfile << line << endl;
    }
    
   // printing the final key to the final synthesized circuit
    outfile << "The secret key is: ";
    for (int i = 0; i < key.size(); i++)
    {
        outfile << key[i];
    }
    outfile << endl;
    
    infile.close();
    outfile.close();
    
    

    return 0;
}


// function to check if element exists in array
            
bool isSimilar(int array[], int size, int element)
  {
      for (int i = 0; i < size; i++)
      {
          if (array[i] == element)
          {
              return true;
          }
      }
    
    return false;
  }

// function to create the vectors for a given circuit
void createVectors(string inputFile,int n) // function takes in filename and key size
{
   
   // opening input circuit file
    ifstream infile;
 
    
    ofstream outfile;
    outfile.open("Vectors_"+inputFile);
    
    if (outfile.fail())
    {
        cerr << "error opening output file" << endl;
        exit(-1);
    }
    
    
    regex keyNum("0|1"); // expression to check for 0s or 1s
    regex keyEx("(0|1)+"); // expression to find the key
    string gate;
    
    
    smatch m,s; // flag type for determining the matching behavior
    

    string line,key,line2; // string variables to read input from the file
    list <string> gates; // the list which would store the gates around each key input
    
   regex input("\\.\\w+\\(\\w+\\)"); // expression that matches to an input/output instance
   int maxInput = 0; // tracks the maximum input size of any gate
   
   string keyInput = "";
    
    string wire,gateType;
   
 
   maxInput = 3; // defining the maxinput to be 3 (disregarding the keyinput)
    
   // initializing all the gate types to NONE
   vector<string> inputs(maxInput);
    for (int i = 0; i < maxInput; i++)
    {
        inputs[i]= "NONE";
    }
    
    vector<string> keyGates; // vector to track the gates associated with each keyinput
    
    bool isGate = false; // boolean variable to track if a gate is found at multiple points
    
    for (int i = 0; i < n; i++) // loop to go through each keygate
    {
       isGate = false; // resets the isGate to false for every instance of n
       keyInput = "KEYINPUT" + to_string(i);
       
       // adding the key gate inputs to the gates list
       
        gate = "\\.\\w+\\(" + keyInput + "\\)"; // string that matches to key input to the keygate
        
       infile.open(inputFile);
       // looks for all the gates associated with the keyinput and adds it to the keyGates vector
       while (!infile.eof())
        {
            getline(infile,line);
            if (regex_search(line,m,regex(gate)))
            {
                if (regex_search(line,m,regex("(U|g)\\d+\\s*\\(")))
                {
                    keyGates.push_back(m[0].str());
                }
            }
        }
        
       infile.close(); // closes the file to reset the file pointer for the upcoming functions
   
        
       // if the keyGates size is 0 because it is just the locked vectors before synthesis with only one gate associated with each key input (the gates dont match to the above defined regex: "(U|g)\\d+\\s*\\("
        if (keyGates.size()==0)
        {
            gates.push_front("KI"+ to_string(i)); // first element of the be
            infile.open(inputFile); // resets the file pointer to the beginning of the file
            
            while(!infile.eof())
            {
                getline(infile,line);
                
                if(regex_search(line,m,regex(gate))) // searches for a gate with keyinput
                {
                    
                   // finding the wires of the other inputs of the key gate
                    sregex_iterator it(line.begin(), line.end(),input);
                    sregex_iterator end;
                    int k = 0;
                    while(it != end)
                    {
                        smatch match = *it;
                        if (regex_match(match.str(),regex("\\.(Z|ZN)\\(\\w+\\)"))) // skips the output wire
                        {
                            ++it;
                            continue;
                        }
                        
                        if (regex_match(match.str(),regex("\\.\\w+\\(KEYINPUT\\d+\\)"))) // skips the keyinput wire
                        {
                            ++it;
                            continue;
                        }
                        
                        inputs[k] = match.str();
                        ++it;
                        ++k;
                    }
                    
                    for (int j = 0; j < maxInput; j++) // goes through all the inputs of the keygate
                    {
                        if (inputs[j] == "NONE") // if no key input found, skips this input and continues to next
                        {
                           gates.push_back("NONE");
                            continue;
                        }
                       
                       // finds the gate that is output for the selected input wire
                        wire =  inputs[j];
                        wire = regex_replace(wire,regex("\\.\\w+"),"");
                        wire = regex_replace(wire,regex("\\(|\\)"),"");
                        
                       infile.close();
                       infile.open(inputFile); // resets the file pointer to the beginning
                        
                       while(!infile.eof())
                        {
                            getline(infile,line2);
                            if (regex_search(line2,s,regex("\\.(Z|ZN)\\(" + wire + "\\)")))
                            {
                                isGate = true;
                                gate = "\\w+_X\\d";
                                regex_search(line2,s,regex(gate));
                                gateType = s[0];
                                gateType = regex_replace(gateType,regex("\\d*_X\\d"),"");
                                gates.push_back(gateType);
                                break;
                            }
                        }
                        
                       // if no output gate found that input wire, it is a primary input
                        if (isGate==false)
                        {
                            gates.push_back("PI");
                        }
                        
                    }
                   
                   // resets inputs to NONE
                   for (int i = 0; i < maxInput; i++)
                   {
                       inputs[i]= "NONE";
                   }
                    
                    // finding the gate of the key gate
                    
                    gate = "\\w+X\\d";
                    regex_search(line,s,regex(gate));
                    gateType = s[0];
                    gateType = regex_replace(gateType,regex("\\d*_X\\d"),"");
                    gates.push_back(gateType);
                    
                    // finding the functionality of the gate at the fan-out of the key-gate.
                   
                   // finds the output wire of the key gate
                    regex_search(line,s,regex("\\.(ZN|Z)\\(\\w+\\)"));
                    wire = s[0];
                    wire = regex_replace(wire,regex("\\.(ZN|Z)"),"");
                    wire = regex_replace(wire,regex("\\(|\\)"),"");
                    infile.close();
                    infile.open(inputFile);
                    infile.seekg(0, ios::beg);
                   
                   // searches which gate is the output wire input to
                    while (!infile.eof())
                    {
                       getline(infile,line2);
                       if (regex_search(line2,s,regex("\\.\\w+\\("+wire+"\\)")))
                       {
                          string gate5 = s[0].str();
                          if (!regex_search(gate5,s,regex("\\.(ZN|Z)\\(")))
                          {
                             isGate =true;
                             gate = "\\w+X\\d";
                             regex_search(line2,s,regex(gate));
                             gateType = s[0];
                             gateType = regex_replace(gateType,regex("\\d*_X\\d"),"");
                             gates.push_back(gateType);
                             
                             // outputs the vector to the output file
                             list<string>::iterator it;
                             for (it = gates.begin(); it != gates.end(); ++it)
                                
                             {
                                outfile << " " << *it;
                             }
                             
                             outfile << endl;
                             gates.pop_back();
                             
                          }
                          
                       }
                    }
                    
                    // if no gates at the fan-out of the key-gate, the output wire is a primary outout
                    if (isGate==false)
                    {
                        gates.push_back("PO");
                       // outputs the vector to the output file
                       list<string>::iterator it;
                         for (it = gates.begin(); it != gates.end(); ++it)
                         
                         
                         {
                         outfile << " " << *it;
                         }
                         
                         outfile << endl;
                        
                    }
                    infile.close();
                }
            }
           
           infile.close();
           gates.clear(); // clears the gates to be used in the next iteration/upcoming loops and funtions
        }
            
        
            for (int h = 0; h < keyGates.size(); h++) // goes through all the gates found for the key input
            {
               isGate = false;
                gates.push_front("KI"+ to_string(i));
            
                    gate = keyGates[h];
                    gate = regex_replace(gate,regex("\\("),"");
                
                infile.open(inputFile);
    
                while(!infile.eof())
                {
                    getline(infile,line);
                    
                   // finding the gate of the other inputs of the key gate
                    if(regex_search(line,m,regex(gate)))
                    {
                        sregex_iterator it(line.begin(), line.end(),input);
                        sregex_iterator end;
                        int k = 0;
                        while(it != end)
                        {
                            smatch match = *it;
                            if (regex_match(match.str(),regex("\\.(Z|ZN)\\(\\w+\\)"))) // skips the output wire
                            {
                                ++it;
                                continue;
                            }
                            
                            if (regex_match(match.str(),regex("\\.\\w+\\(KEYINPUT\\d+\\)"))) // skips the keyinput
                            {
                                ++it;
                                continue;
                            }
                            
                            inputs[k] = match.str();
                            ++it;
                            ++k;
                        }
                        
                        for (int j = 0; j < maxInput; j++) // goes through all the inputs of the keygate
                        {
                            
                            if (inputs[j] == "NONE") // if no key input found, skips this input and continues to next
                            {
                                gates.push_back("NONE");
                                continue;
                            }
                           
                           // finds the gate that is output for the selected input wire
                            wire =  inputs[j];
                            wire = regex_replace(wire,regex("\\.\\w+"),"");
                            wire = regex_replace(wire,regex("\\(|\\)"),"");
                            
                           infile.close();
                           infile.open(inputFile); // resets the file pointer to the beginning
                            
                           while(!infile.eof())
                            {
                                getline(infile,line2);
                                if (regex_search(line2,s,regex("\\.(Z|ZN)\\(" + wire + "\\)")))
                                {
                                    isGate = true;
                                    gate = "\\w+_X\\d";
                                    regex_search(line2,s,regex(gate));
                                    gateType = s[0];
                                    gateType = regex_replace(gateType,regex("\\d*_X\\d"),"");
                                    gates.push_back(gateType);
                                    break;
                                }
                            }
                           
                           // if no output gate found that input wire, it is a primary input
                            if (isGate==false)
                            {
                                gates.push_back("PI");
                            }
                            
                        }
                       
                       // set inputs to NONE
                       for (int i = 0; i < maxInput; i++)
                       {
                           inputs[i]= "NONE";
                       }
                       
                       // finding the gate of the key gate
                        
                        gate = "\\w+X\\d";
                        regex_search(line,s,regex(gate));
                        gateType = s[0];
                        gateType = regex_replace(gateType,regex("\\d*_X\\d"),"");
                        gates.push_back(gateType);
                        
                        // finding the functionality of the gate at the fan-out of the key-gate.
                       
                       // finds the output wire of the key gate
                        regex_search(line,s,regex("\\.(ZN|Z)\\(\\w+\\)"));
                        wire = s[0];
                        wire = regex_replace(wire,regex("\\.(ZN|Z)"),"");
                        wire = regex_replace(wire,regex("\\(|\\)"),"");
                        infile.close();
                        infile.open(inputFile);
                        infile.seekg(0, ios::beg);
                       
                       // searches which gate is the output wire input to
                       while (!infile.eof())
                        {
                            getline(infile,line2);
                           if (regex_search(line2,s,regex("\\.\\w+\\("+wire+"\\)")))
                           {
                              string gate5 = s[0].str();
                              if (!regex_search(gate5,s,regex("\\.(ZN|Z)\\(")))
                              {
                                 isGate =true;
                                 gate = "\\w+X\\d";
                                 regex_search(line2,s,regex(gate));
                                 gateType = s[0];
                                 gateType = regex_replace(gateType,regex("\\d*_X\\d"),"");
                                 gates.push_back(gateType);
                                 
                                 // outputs the vector to the output file
                                 list<string>::iterator it;
                                 for (it = gates.begin(); it != gates.end(); ++it)
                                    
                                 {
                                    outfile << " " << *it;
                                 }
                                 
                                 outfile << endl;
                                 gates.pop_back();
                                 
                              }
                           }
                        }
                        
                       // if no gates at the fan-out of the key-gate, the output wire is a primary outout
                        if (isGate==false)
                        {
                            gates.push_back("PO");
                           
                           // outputs the vector to the output file
                            list<string>::iterator it;
                            for (it = gates.begin(); it != gates.end(); ++it)
                                
                                
                            {
                                outfile << " " << *it;
                            }
                            
                            outfile << endl;
                            
                        }
                    }
                }
               infile.close();
                gates.clear(); // clears the gates to be used in the next iteration for the next key gate associated with the key input
            }
            keyGates.clear(); // clears the gates to be used in the next iteration of the key input
           // gates.clear(); // clears the gates to be used in the next iteration for the next key gate associated with the next key input
        }

    cout << "Vectors created successfully for " << inputFile << "!" << endl;
}


