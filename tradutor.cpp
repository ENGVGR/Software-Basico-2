#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>
#include <cstdlib>
#include <map>
#include <vector>

using namespace std;

map<int, string> instructions = {{1, "ADD"}, {2, "SUB"}, {3, "MUL"}, {4, "DIV"}, {5, "JMP"}, {6, "JMPN"}, {7, "JMPP"}, {8, "JMPZ"}, {9, "COPY"}, {10, "LOAD"}, {11, "STORE"}, {12, "INPUT"}, {13, "OUTPUT"}, {14, "STOP"}, {15, "S_INPUT"}, {16, "S_OUTPUT"}};

vector<int> numbers;

void translator(string input_file_name, string output_file_name)
{
  bool can_write = false;

  string line_to_read = "";

  vector<string> source_code;

  ofstream output_file(output_file_name);

  if (!output_file.is_open())
  {
    cerr << "Erro ao abrir o arquivo: " << output_file_name << endl;
    exit(1);
  }

  ifstream input_file(input_file_name);

  if (!input_file.is_open())
  {
    cerr << "Erro ao abrir o arquivo: " << input_file_name << endl;
    exit(1);
  }

  // Lê o arquivo linha por linha
  while (getline(input_file, line_to_read))
  {
    istringstream line_readed(line_to_read);

    int number_readed;

    while (line_readed >> number_readed)
    {
      numbers.push_back(number_readed);
    }

    size_t index = 0;

    while (index < numbers.size())
    {
      int current_number = numbers[index];

      switch (current_number)
      {
      case 0:
        break;
        
      /* Copy */
      case 9:
        index++;
        output_file << instructions[current_number] + " " + to_string(numbers[index]) + " ";
        index++;
        output_file << to_string(numbers[index]) << endl;
        break;

      /* Input */
      case 12:
        index++;
        output_file << "push dword " + to_string(numbers[index]) << endl;
        output_file << "call input" << endl;
        break;

      /* Output */
      case 13:
        index++;
        output_file << "push dword " + to_string(numbers[index]) << endl;
        output_file << "call output" << endl;
        break;

      /* Stop */
      case 14:
        output_file << instructions[current_number] << endl;
        break;

      /* S_Input */
      case 15:
        index++;
        output_file << "push dword " + to_string(numbers[index]) << endl;
        index++;
        output_file << "push dword " + to_string(numbers[index]) << endl;
        output_file << "call s_input" << endl;
        break;

      /* S_Output */
      case 16:
        index++;
        output_file << "push dword " + to_string(numbers[index]) << endl;
        index++;
        output_file << "push dword " + to_string(numbers[index]) << endl;
        output_file << "call s_output" << endl;
        break;

      default:
        int extra_number_readed;
        index++;
        output_file << instructions[current_number] + " " + to_string(numbers[index]) << endl;
        break;
      }

      index++;
    }

    input_file.close();
    output_file.close();
  }
}

int main(int argc, char *argv[])
{
  string output_file_name;

  string argument = argv[1];

  output_file_name = argument.substr(0, argument.length() - 4) + ".s";
  translator(argument, output_file_name);

  return 0;
}