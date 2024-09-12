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

string dat = "section .data\n"
              "overflow_msg db 'Erro: Overflow na multiplicacao!', 0xA\n"
              "overflow_len equ $ - overflow_msg\n"
              "buffer dd 0,0,0,0,0,0,0,0,0,0,0\n"
              "newline db 0xA, 0\n";

string bss = "section .bss\n";

string final_text = "section .text\n"
                    "global _start\n\n"
                    "output:\n"
                    "enter 0, 0                ; Salva o frame da pilha\n"
                    "pusha\n"

                    "mov eax, [ebp + 8]\n"
                    "mov eax, [eax]\n"

                    "mov ecx, 10                 ; Divisor para extração de dígitos (base 10)\n"
                    "lea esi, [buffer + 11]  ; Aponta para o final do buffer\n"
                    "mov byte [esi], 0           ; Coloca o terminador nulo no final da string\n"
                    "dec esi                     ; Move o ponteiro para a posição anterior no buffer\n\n"

                    "convert_loop:\n"

                    "xor edx, edx          ; Limpa edx antes da divisão (necessário para div)\n"
                    "div ecx               ; Divide EAX por 10, quociente em EAX, resto em EDX\n"
                    "add dl, 0x30         ; Converte o dígito (resto) para ASCII\n"
                    "mov [esi], dl         ; Armazena o dígito convertido no buffer\n"
                    "dec esi               ; Move o ponteiro para o próximo caractere\n"
                    "cmp eax, 0            ; Verifica se o quociente é 0\n"
                    "jne convert_loop      ; Se EAX não for 0, continua a conversão\n"

                    "inc esi               ; Corrige o ponteiro para o início da string\n"

                    "mov eax, [buffer]\n"
                    "mov dword [buffer], eax\n"
                    "mov eax, 4\n"
                    "mov ebx, 1\n"
                    "mov ecx, buffer\n"
                    "mov edx, 21\n"
                    "int 0x80\n"

                    "mov eax, 4\n"
                    "mov ebx, 1\n"
                    "mov ecx, newline\n"
                    "mov edx, 1\n"
                    "int 0x80\n"

                    "mov ecx, 11\n"
                    "mov eax, 0\n\n"
                    "clear1:\n"
                    "mov dword[buffer+eax*4],0\n"
                    "inc eax\n"
                    "loop clear1\n"
                    "popa\n"
                    "leave\n"
                    "ret                   ; Retorna com a string pronta no buffer \n\n"

                    "input:\n"
                    "%define ATUAL dword[ebp - 4]\n"
                    "%define RES dword[ebp - 8]\n"

                    "enter 4,0\n"
                    "pusha\n"
                    "mov RES, 0\n"

                    "mov eax, 3\n"
                    "mov ebx, 0\n"
                    "mov ecx, buffer      ;salva o input como uma string em buffer\n"
                    "mov edx, 32\n"
                    "int 0x80\n"

                    "mov edx, 0\n\n"
                    "loop:\n"
                    "mov al, [buffer + edx]         ; Carrega o caractere do buffer em al (parte baixa de eax)\n"

                    "cmp al, 0x0A             ; Verifica se o caractere é Enter (0x0A)\n"
                    "je saida                 ; Se for Enter, sai do loop\n"

                    "; Converter o caractere lido de ASCII para valor numérico\n"
                    "sub al, 0x30              ; Subtrai '0' (0x30) para converter para decimal\n"
                    "movzx eax, al            ; Expande al para eax sem sinal (agora temos o valor numérico)\n"

                    "; Atualiza o valor total em N\n"
                    "mov ebx, RES             ; Carrega o valor atual de N\n"
                    "mov ATUAL,ebx\n"
                    "mov ecx ,9\n\n"
                    "multi:\n"
                    "add ebx,ATUAL\n"
                    "loop multi\n"
                    "add ebx, eax             ; Soma o novo dígito\n"
                    " mov RES, ebx             ; Salva o novo valor de N\n"

                    "  inc edx\n"
                    "   jmp loop                 ; Repete o loop\n\n"

                    "saida:\n"
                    "mov ecx, 11\n"
                    "mov eax, 0\n"
                    "clear2:\n"
                    "mov dword[buffer+eax*4],0\n"
                    "inc eax\n"
                    "loop clear2\n"
                    "popa\n"
                    "mov eax, RES\n"
                    "mov dword[ebp + 8], eax\n"
                    "leave\n"
                    "ret                   ; Retorna com a string pronta no buffer\n\n"
                    "_start:\n";

map<int, string> text;

/* Endereço da label no assembly inventado, Nome da label  */
map<int, string> labels_list = {};

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

    /* Encontra o STOP */
    while (index < numbers.size())
    {
      int current_number = numbers[index];
      if (current_number == 14)
      {
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

      if (current_number == 0)
      {
        bss += "LABEL" + to_string(label_number) + " resb 4\n"; /* 4 bytes de memória (32 bits) */
      }
      else
      {
        dat += "LABEL" + to_string(label_number) + " dd " + to_string(current_number) + "\n";
      }

      index++;
      label_number++;
      contador++;
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
        if (variable_list.find(numbers[index]) != variable_list.end())
        {
          text[contador] = "MOV EBX, [" + variable_list[numbers[index]] + "]\nADD EAX, EBX\n";
        }
        else
        {
          text[contador] = "MOV EBX, " + to_string(numbers[index]) + "\nADD EAX, EBX\n";
        }
        contador += 2;
        break;

      /* SUB */
      case 2:
        index++;
        if (variable_list.find(numbers[index]) != variable_list.end())
        {
          text[contador] = "MOV EBX, [" + variable_list[numbers[index]] + "]\nSUB EAX, EBX\n";
        }
        else
        {
          text[contador] = "MOV EBX, " + to_string(numbers[index]) + "\nSUB EAX, EBX\n";
        }
        contador += 2;
        break;

      /* MUL */
      case 3:
        index++;
        if (variable_list.find(numbers[index]) != variable_list.end())
        {
          text[contador] = "MOV EBX, [" + variable_list[numbers[index]] + "]\nIMUL EBX\nCMP EDX, 0\nJNE overflow_handler\n";
        }
        else
        {
          text[contador] = "MOV EBX, " + to_string(numbers[index]) + "\nIMUL EBX\nCMP EDX, 0\nJNE overflow_handler\n";
        }
        contador += 2;
        break;

      /* DIV */
      case 4:
        index++;
        if (variable_list.find(numbers[index]) != variable_list.end())
        {
          text[contador] = "MOV EBX, [" + variable_list[numbers[index]] + "]\nXOR EDX, EDX\nIDIV EBX \n";
        }
        else
        {
          text[contador] = "MOV EBX, " + to_string(numbers[index]) + "\nXOR EDX, EDX\nIDIV EBX \n";
        }
        contador += 2;
        break;

      /* JMP */
      case 5:
        index++;
        if (labels_list.find(numbers[index]) != labels_list.end())
        {
          text[contador] = "JMP " + labels_list[numbers[index]] + "\n";
        }
        else
        {
          text[contador] = "JMP LABEL" + to_string(label_number) + "\n";
          labels_list[numbers[index]] = "LABEL" + to_string(label_number);
          label_number++;
        }
        contador += 2;
        break;

      /* JMPN */
      case 6:
        index++;
        if (labels_list.find(numbers[index]) != labels_list.end())
        {
          text[contador] = "CMP EAX, 0\n";
          text[contador] += "JL " + labels_list[numbers[index]] + "\n";
        }
        else
        {
          text[contador] = "CMP EAX, 0\n";
          text[contador] += "JL LABEL" + to_string(label_number) + "\n";
          labels_list[numbers[index]] = "LABEL" + to_string(label_number);
          label_number++;
        }
        contador += 2;
        break;

      /* JMPP */
      case 7:
        index++;
        if (labels_list.find(numbers[index]) != labels_list.end())
        {
          text[contador] = "CMP EAX, 0\n";
          text[contador] += "JG " + labels_list[numbers[index]] + "\n";
        }
        else
        {
          text[contador] = "CMP EAX, 0\n";
          text[contador] += "JG LABEL" + to_string(label_number) + "\n";
          labels_list[numbers[index]] = "LABEL" + to_string(label_number);
          label_number++;
        }
        contador += 2;
        break;

      /* JMPZ */
      case 8:
        index++;
        if (labels_list.find(numbers[index]) != labels_list.end())
        {
          text[contador] = "CMP EAX, 0\n";
          text[contador] += "JE " + labels_list[numbers[index]] + "\n";
        }
        else
        {
          text[contador] = "CMP EAX, 0\n";
          text[contador] += "JE LABEL" + to_string(label_number) + "\n";
          labels_list[numbers[index]] = "LABEL" + to_string(label_number);
          label_number++;
        }
        contador += 2;
        break;

      /* Copy */
      case 9:
        index++;
        if (variable_list.find(numbers[index]) != variable_list.end())
        {
          text[contador] = "MOV EDX, [" + variable_list[numbers[index]] + "]\n";
        }
        else
        {
          text[contador] = "MOV EDX, [" + to_string(numbers[index]) + "]\n";
        }

        if (variable_list.find(numbers[index+1]) != variable_list.end())
        {
          text[contador] += "MOV DWORD [" + variable_list[numbers[index+1]] + "], EDX\n";
        }
        else
        {
          text[contador] += "MOV DWORD [" + to_string(numbers[index + 1]) + "], EDX\n";
        }
        index++;
        contador += 3;
        break;

      /* LOAD */
      case 10:
        index++;
        if (variable_list.find(numbers[index]) != variable_list.end())
        {
          text[contador] = "MOV EAX, [" + variable_list[numbers[index]] + "]\n";
        }
        else
        {
          text[contador] = "MOV EAX, " + to_string(numbers[index]) + "\n";
        }
        contador += 2;
        break;

      /* STORE */
      case 11:
        index++;
        if (variable_list.find(numbers[index]) != variable_list.end())
        {
          text[contador] = "MOV DWORD [" + variable_list[numbers[index]] + "], EAX\n";
        }
        else
        {
          text[contador] = "MOV " + to_string(numbers[index]) + ", EAX\n";
        }
        contador += 2;
        break;

      /* Input */
      case 12:
        index++;
        if (variable_list.find(numbers[index]) != variable_list.end())
        {
          text[contador] = "PUSH " + variable_list[numbers[index]] + "\nCALL input\nPOP DWORD [" + variable_list[numbers[index]] + "]\n";
        }
        else
        {
          text[contador] = "PUSH " + to_string(numbers[index]) + "\nCALL input\nPOP DWORD [" + variable_list[numbers[index]] + "]\n";
        }
        contador += 2;
        break;

      /* Output */
      case 13:
        index++;
        if (variable_list.find(numbers[index]) != variable_list.end())
        {
          text[contador] = "PUSH " + variable_list[numbers[index]] + "\nCALL output\nPOP ECX\n";
        }
        else
        {
          text[contador] = "PUSH " + to_string(numbers[index]) + "\nCALL output\nPOP ECX\n";
        }
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
        contador += 3;
        break;

      /* S_Output */
      case 16:
        index++;
        index++;
        contador += 3;
        break;

      default:
        break;
      }

      index++;
      if (current_number == 14)
      {
        break;
      }
    }

    /* Adicionando as Labels no código */
    for (auto it = labels_list.begin(); it != labels_list.end(); ++it)
    {
      text[it->first] = it->second + ": " + text[it->first];
    }

    /* Adicionando as Labels no código */
    for (auto it = text.begin(); it != text.end(); ++it)
    {
      final_text += it->second;
    }

    final_text +=
        "\noverflow_handler:\n"
        "    MOV EAX, 4\n"            /*sys_write */
        "    MOV EBX, 1\n"            /* stdout */
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

    output_file << dat << endl;
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