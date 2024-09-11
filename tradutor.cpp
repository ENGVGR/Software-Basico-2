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

string data = ".data\n"
"overflow_msg db 'Erro: Overflow na multiplicacao!', 0xA\n"
"overflow_len equ $ - overflow_msg\n";

string bss = ".bss\n";
string final_text = ".text\n_start:\n";
map<int, string> text;

/* Endereço da label no assembly inventado, Nome da label  */
map<int, string> labels_list = {};
/* Endereço da label no ia-32, Nome da label */
map<int, string> labels_list_end = {};

/* Endereço de memória, Nome da variável */
map<int, string> variable_list = {};

int label_number = 1;
int contador = 0;

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

    /* APAGAR !!! */
    while (index < numbers.size())
    {
      int current_number = numbers[index];
      cout << instructions[current_number] << endl;
      if (current_number == 14){
        break;
      }
      switch (current_number)
      {
      case 0:
        break;
        
      /* Copy */
      case 9:
        index += 2;
        contador += 3;
        break;

      /* Stop */
      case 14:
      contador += 1;
        break;

      /* S_Input */
      case 15:
        index += 2;
        contador += 3;
        break;

      /* S_Output */
      case 16:
        index += 2;
        contador += 3;
        break;

      default:
        index++;
        contador += 2;
        break;
      }
      index++;
    }

    index = 0;
    
    /* Encontra o STOP */
     while (index < numbers.size())
    {
      int current_number = numbers[index];
      if (current_number == 14){
        break;
      }
      switch (current_number)
      {
      case 0:
        break;
        
      /* Copy */
      case 9:
        index += 2;
        break;

      /* Stop */
      case 14:
        break;

      /* S_Input */
      case 15:
        index += 2;
        break;

      /* S_Output */
      case 16:
        index += 2;
        break;

      default:
        index++;
        break;
      }

      index++;
    }

    index++;

    /* Depois do STOP */
    while (index < numbers.size())
    {
      int current_number = numbers[index];

      variable_list[index] = "LABEL" + to_string(label_number);

      if (current_number == 0){
        bss += "LABEL" + to_string(label_number) + " resb 4\n"; /* 4 bytes de memória (32 bits) */
      } else {
        data += "LABEL" + to_string(label_number) + " dd " + to_string(current_number) + "\n";
      }

      index++;
      label_number++;
      contador++;
    }

    cout << "Lista de variaveis: " << endl;
    for (auto it = variable_list.begin(); it != variable_list.end(); ++it) {
        cout << to_string(it->first) + ": " + it->second << endl;
    }

    index = 0;
    contador = 0;

    while (index < numbers.size())
    {
      int current_number = numbers[index];

      switch (current_number)
      {
      case 0:
        break;

      /* ADD */
      case 1:
        index++;
        if (variable_list.find(numbers[index]) != variable_list.end()) {
          text[contador] = "MOV EBX, [" + variable_list[numbers[index]] + "]\nADD EAX, EBX\n";
        } else {
          text[contador] = "MOV EBX, " + to_string(numbers[index]) + "\nADD EAX, EBX\n";
        }
        contador += 2;
        break;

      /* SUB */
      case 2:
        index++;
        if (variable_list.find(numbers[index]) != variable_list.end()) {
          text[contador] = "MOV EBX, [" + variable_list[numbers[index]] + "]\nSUB EAX, EBX\n";
        } else {
          text[contador] = "MOV EBX, " + to_string(numbers[index]) + "\nSUB EAX, EBX\n";
        }
        contador += 2;
        break;

      /* MUL */
      case 3:
        index++;
        if (variable_list.find(numbers[index]) != variable_list.end()) {
          text[contador] = "MOV EBX, [" + variable_list[numbers[index]] + "]\nIMUL EBX\nCMP IDX, 0\nJNE overflow_handler\n";
        } else {
          text[contador] = "MOV EBX, " + to_string(numbers[index]) + "\nIMUL EBX\nCMP IDX, 0\nJNE overflow_handler\n";
        }
        contador += 2;
        break;
      
      /* DIV */
      case 4:
        index++;
        if (variable_list.find(numbers[index]) != variable_list.end()) {
          text[contador] = "MOV EBX, [" + variable_list[numbers[index]] + "]\nXOR EDX, EDX\nIDIV EBX \n";
        } else {
          text[contador] = "MOV EBX, " + to_string(numbers[index]) + "\nXOR EDX, EDX\nIDIV EBX \n";
        }
        contador += 2;
        break;
      
      /* JMP */
      case 5:
        index++;
        if (labels_list.find(numbers[index]) != labels_list.end()) {
          text[contador] = "JMP " + labels_list[numbers[index]] + "\n";
        } else {
          text[contador] = "JMP LABEL" + to_string(label_number) + "\n";
          labels_list[numbers[index]] = "LABEL" + to_string(label_number);
          label_number++;
        }
        contador += 2;
        break;
      
      /* JMPN */
      case 6:
        index++;
        if (labels_list.find(numbers[index]) != labels_list.end()) {
          text[contador] = "JB " + labels_list[numbers[index]] + "\n";
        } else {
          text[contador] = "JB LABEL" + to_string(label_number) + "\n";
          labels_list[numbers[index]] = "LABEL" + to_string(label_number);
          label_number++;
        }
        contador += 2;
        break;
      
      /* JMPP */
      case 7:
        index++;
        if (labels_list.find(numbers[index]) != labels_list.end()) {
          text[contador] = "JA " + labels_list[numbers[index]] + "\n";
        } else {
          text[contador] = "JA LABEL" + to_string(label_number) + "\n";
          labels_list[numbers[index]] = "LABEL" + to_string(label_number);
          label_number++;
        }
        contador += 2;
        break;

      /* JMPZ */
      case 8:
        index++;
        if (labels_list.find(numbers[index]) != labels_list.end()) {
          text[contador] = "JE " + labels_list[numbers[index]] + "\n";
        } else {
          text[contador] = "JE LABEL" + to_string(label_number) + "\n";
          labels_list[numbers[index]] = "LABEL" + to_string(label_number);
          label_number++;
        }
        contador += 2;
        break;

      /* Copy */
      case 9:
        index++;
        text[contador] = "MOV " + to_string(numbers[index+1]) + "," + to_string(numbers[index]) + "\n";
        index++;
        contador += 3;
        break;

      /* LOAD */
      case 10:
        index++;
        if (variable_list.find(numbers[index]) != variable_list.end()) {
          text[contador] = "MOV EAX, " + variable_list[numbers[index]] + "\n";
        } else {
          text[contador] = "MOV EAX, " + to_string(numbers[index]) + "\n";
        }
        contador += 2;
        break;

      /* STORE */
      case 11:
        index++;
        if (variable_list.find(numbers[index]) != variable_list.end()) {
          text[contador] = "MOV " + variable_list[numbers[index]] + ", EAX\n";
        } else {
          text[contador] = "MOV " + to_string(numbers[index]) + ", EAX\n";
        }
        contador += 2;
        break;

      /* Input */
      case 12:
        index++;
        text[contador] = "call input\n";
        contador += 2;
        break;

      /* Output */
      case 13:
        index++;
        text[contador] = "call output\n";
        contador += 2;
        break;

      /* Stop */
      case 14:
        text[contador] = "JMP END\n";
        contador += 1;
        break;

      /* S_Input */
      case 15:
        index++;
        index++;
        text[contador] = "call s_input\n";
        contador += 3;
        break;

      /* S_Output */
      case 16:
        index++;
        index++;
        text[contador] = "call s_output\n";
        contador += 3;
        break;

      default:
        break;
      }

      index++;
      if (current_number == 14){
        break;
      }
    }

    /* Adicionando as Labels no código */
    for (auto it = labels_list.begin(); it != labels_list.end(); ++it) {
        text[it->first] = it->second + ": " + text[it->first];
    }

    /* Adicionando as Labels no código */
    for (auto it = text.begin(); it != text.end(); ++it) {
        final_text += it->second;
    }

    final_text +=
        "overflow_handler:\n"
        "    MOV EAX, 4\n" /*sys_write */
        "    MOV EBX, 1\n" /* stdout */
        "    MOV ECX, overflow_msg\n" /* mensagem de erro */
        "    MOV EDX, overflow_len\n" /* Comprimento da mensagem */
        "    INT 0x80\n"
        "    MOV EAX, 1\n" /* sys_exit */
        "    MOV EBX, 1\n" /* Código de erro (1) */
        "    INT 0x80\n";

        final_text +=
        "END:\n"
        "    MOV EAX, 1\n" /* sys_exit */
        "    MOV EBX, 0\n"
        "    INT 0x80\n";

    output_file << data << endl;
    output_file << bss << endl;
    output_file << final_text << endl;

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