//
//  main.cpp
//  VectorEncoding
//
//  Created by Fatima Farooq on 19/07/2023.
//

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <regex>
#include <list>

using namespace std;

int main() {
    
    string inputFile; // input file name
    int n; // key size
    
    cout << "Enter file name" << endl;
    cin >> inputFile;

    cout << "Enter key size" << endl;
    cin >> n;
    
    
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
    

    return 0;
}
