#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

// Some helpful constants
#define MAX_LINE_LENGTH 256
#define MAX_LABELS 1000
#define MAX_INSTRUCTIONS 8192
#define MAX_LABEL_LENGTH 64

// Hepful global variable for error handling
static int current_line = 0;

typedef struct {
    char name[MAX_LABEL_LENGTH];
    int address;
} Symbol;

typedef struct {
    Symbol symbols[MAX_LABELS];
    int num_symbols;
    uint16_t machine_code[MAX_INSTRUCTIONS];
    int num_instructions;
} Assembler;

void error(const char* msg, int line_num) {
    if (line_num > 0) {
        fprintf(stderr, "Error at line %d: %s\n", line_num, msg);
    } 
    else {
        fprintf(stderr, "Error: %s\n", msg);
    }
    exit(1);
}

void strip_comment(char* line) {
    char* comment = strchr(line, '#');
    if (comment) {
        *comment = '\0';
    }
}

int is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

int tokenize(char* line, char tokens[][MAX_LABEL_LENGTH], int max_tokens) {
    int count = 0;
    char* ptr = line;
    
    while (*ptr && count < max_tokens) {
        while (*ptr && (is_whitespace(*ptr) || *ptr == ',')) {
            ptr++;
        }
        if (!*ptr) {
            break;
        }

        // Extract token
        int i = 0;
        while (*ptr && !is_whitespace(*ptr) && *ptr != ',' && i < MAX_LABEL_LENGTH - 1) {
            tokens[count][i++] = *ptr++;
        }
        tokens[count][i] = '\0';
        
        if (tokens[count][0] != '\0') {
            count++;
        }
    }
    return count;
}

int is_valid_label(const char* label) {
    if (!label || !*label || (!isalpha(label[0]) && label[0] != '_')) {
        return 0;
    }

    for (int i = 1; label[i]; i++) {
        if (!isalnum(label[i]) && label[i] != '_') {
            return 0;
        }
    }
    return 1;
}

void to_lowercase(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

int find_symbol(Assembler* as, const char* name) {
    char lower_name[MAX_LABEL_LENGTH];
    strncpy(lower_name, name, MAX_LABEL_LENGTH - 1);
    lower_name[MAX_LABEL_LENGTH - 1] = '\0';
    to_lowercase(lower_name);
    
    for (int i = 0; i < as->num_symbols; i++) {
        if (strcmp(as->symbols[i].name, lower_name) == 0) {
            return as->symbols[i].address;
        }
    }
    return -1;
}

void add_symbol(Assembler* as, const char* name, int address) {
    if (as->num_symbols >= MAX_LABELS) {
        error("Too many labels", current_line);
    }
    
    char lower_name[MAX_LABEL_LENGTH];
    strncpy(lower_name, name, MAX_LABEL_LENGTH - 1);
    lower_name[MAX_LABEL_LENGTH - 1] = '\0';
    to_lowercase(lower_name);
    
    // Check for duplicates
    if (find_symbol(as, lower_name) != -1) {
        char msg[128];
        sprintf(msg, "Duplicate label: %s", name);
        error(msg, current_line);
    }
    
    strncpy(as->symbols[as->num_symbols].name, lower_name, MAX_LABEL_LENGTH - 1);
    as->symbols[as->num_symbols].address = address;
    as->num_symbols++;
}

int parse_register(const char* reg_str) {
    if (reg_str[0] != '$') {
        char msg[128];
        sprintf(msg, "Invalid register: %s", reg_str);
        error(msg, current_line);
    }
    
    int reg_num = atoi(reg_str + 1);
    if (reg_num < 0 || reg_num > 7) {
        char msg[128];
        sprintf(msg, "Register out of range: %s", reg_str);
        error(msg, current_line);
    }
    return reg_num;
}

int parse_immediate(Assembler* as, const char* imm_str) {
    char lower_imm[MAX_LABEL_LENGTH];
    strncpy(lower_imm, imm_str, MAX_LABEL_LENGTH - 1);
    lower_imm[MAX_LABEL_LENGTH - 1] = '\0';
    to_lowercase(lower_imm);
    
    int addr = find_symbol(as, lower_imm);
    if (addr != -1) {
        return addr;
    }
    
    // Parse as number (handles neg & pos)
    if (isdigit(lower_imm[0]) || lower_imm[0] == '-' || lower_imm[0] == '+') {
        return atoi(lower_imm);
    }
    
    char msg[128];
    sprintf(msg, "Undefined label or invalid immediate: %s", imm_str);
    error(msg, current_line);
    return 0;
}

void parse_memory_ref(Assembler* as, const char* mem_ref, int* imm, int* reg) {
    char buffer[MAX_LABEL_LENGTH];
    strncpy(buffer, mem_ref, MAX_LABEL_LENGTH - 1);
    buffer[MAX_LABEL_LENGTH - 1] = '\0';
    char* paren = strchr(buffer, '(');
    if (!paren) {
        char msg[128];
        sprintf(msg, "Invalid memory reference: %s", mem_ref);
        error(msg, current_line);
    }
    
    *paren = '\0';
    char* reg_part = paren + 1;
    char* close_paren = strchr(reg_part, ')');
    if (close_paren) {
        *close_paren = '\0';
    }
    
    *imm = parse_immediate(as, buffer);
    *reg = parse_register(reg_part);
}

uint16_t encode_signed(int value, int bits) {
    int max_val = (1 << (bits - 1)) - 1;
    int min_val = -(1 << (bits - 1));
    
    if (value < min_val || value > max_val) {
        char msg[128];
        sprintf(msg, "Immediate value %d out of range for %d-bit signed (range: %d to %d)",
                value, bits, min_val, max_val);
        error(msg, current_line);
    }
    if (value < 0) {
        value = (1 << bits) + value;
    }
    return (uint16_t)(value & ((1 << bits) - 1));
}

uint16_t encode_unsigned(int value, int bits) {
    int max_val = (1 << bits) - 1;
    
    if (value < 0 || value > max_val) {
        char msg[128];
        sprintf(msg, "Value %d out of range for %d-bit unsigned (range: 0 to %d)",
                value, bits, max_val);
        error(msg, current_line);
    }
    
    return (uint16_t)(value & ((1 << bits) - 1));
}

uint16_t encode_three_reg(const char* opcode, char operands[][MAX_LABEL_LENGTH], int num_ops) {
    if (num_ops != 3) {
        char msg[128];
        sprintf(msg, "%s requires 3 operands", opcode);
        error(msg, current_line);
    }
    
    int reg_dst = parse_register(operands[0]);
    int reg_src_a = parse_register(operands[1]);
    int reg_src_b = parse_register(operands[2]);
    
    uint16_t suffix = 0;
    if (strcmp(opcode, "add") == 0) {
        suffix = 0x0;
    }
    else if (strcmp(opcode, "sub") == 0) {
        suffix = 0x1;
    }
    else if (strcmp(opcode, "or") == 0) {
        suffix = 0x2;
    }
    else if (strcmp(opcode, "and") == 0) {
        suffix = 0x3;
    }
    else if (strcmp(opcode, "slt") == 0) {
        suffix = 0x4;
    }
    else {
        char msg[128];
        sprintf(msg, "Unknown three-reg opcode: %s", opcode);
        error(msg, current_line);
    }
    
    uint16_t instr = 0;
    instr |= (0 << 13);
    instr |= (reg_src_a << 10);
    instr |= (reg_src_b << 7);
    instr |= (reg_dst << 4);
    instr |= suffix;
    
    return instr;
}

uint16_t encode_jr(char operands[][MAX_LABEL_LENGTH], int num_ops) {
    if (num_ops != 1) {
        error("jr requires 1 operand", current_line);
    }
    
    int reg = parse_register(operands[0]);
    
    uint16_t instr = 0;
    instr |= (0 << 13);
    instr |= (reg << 10);
    instr |= (0 << 7);
    instr |= (0 << 4);
    instr |= 0x8;
    
    return instr;
}

uint16_t encode_two_reg(Assembler* as, const char* opcode, char operands[][MAX_LABEL_LENGTH], int num_ops, int pc) {
    uint16_t opcode_bits;
    
    if (strcmp(opcode, "addi") == 0) {
        opcode_bits = 0x1;
    }
    else if (strcmp(opcode, "lw") == 0) {
        opcode_bits = 0x4;
    }
    else if (strcmp(opcode, "sw") == 0) {
        opcode_bits = 0x5;
    }
    else if (strcmp(opcode, "jeq") == 0) {
        opcode_bits = 0x6;
    }
    else if (strcmp(opcode, "slti") == 0) {
        opcode_bits = 0x7;
    }
    else {
        char msg[128];
        sprintf(msg, "Unknown two-reg opcode: %s", opcode);
        error(msg, current_line);
        return 0;
    }
    
    uint16_t instr = opcode_bits << 13;
    if (strcmp(opcode, "addi") == 0 || strcmp(opcode, "slti") == 0) {
        if (num_ops != 3) {
            char msg[128];
            sprintf(msg, "%s requires 3 operands", opcode);
            error(msg, current_line);
        }
        
        int reg_dst = parse_register(operands[0]);
        int reg_src = parse_register(operands[1]);
        int imm = parse_immediate(as, operands[2]);
        
        instr |= (reg_src << 10);
        instr |= (reg_dst << 7);
        instr |= encode_signed(imm, 7);
        
    } else if (strcmp(opcode, "lw") == 0 || strcmp(opcode, "sw") == 0) {
        if (num_ops != 2) {
            char msg[128];
            sprintf(msg, "%s requires 2 operands", opcode);
            error(msg, current_line);
        }
        
        int reg_data = parse_register(operands[0]);
        int imm, reg_addr;
        parse_memory_ref(as, operands[1], &imm, &reg_addr);
        instr |= (reg_addr << 10);
        instr |= (reg_data << 7);
        instr |= encode_signed(imm, 7);
        
    } else if (strcmp(opcode, "jeq") == 0) {
        if (num_ops != 3) {
            error("jeq requires 3 operands", current_line);
        }
        
        int reg_a = parse_register(operands[0]);
        int reg_b = parse_register(operands[1]);
        int target = parse_immediate(as, operands[2]);
        
        // Calculate relative offset
        int rel_imm = target - (pc + 1);
        instr |= (reg_a << 10);
        instr |= (reg_b << 7);
        instr |= encode_signed(rel_imm, 7);
    }
    
    return instr;
}

uint16_t encode_jump(Assembler* as, const char* opcode, char operands[][MAX_LABEL_LENGTH], int num_ops) {
    if (num_ops != 1) {
        char msg[128];
        sprintf(msg, "%s requires 1 operand", opcode);
        error(msg, current_line);
    }
    
    uint16_t opcode_bits;
    if (strcmp(opcode, "j") == 0) {
        opcode_bits = 0x2;
    }
    else if (strcmp(opcode, "jal") == 0) {
        opcode_bits = 0x3;
    }
    else {
        char msg[128];
        sprintf(msg, "Unknown jump opcode: %s", opcode);
        error(msg, current_line);
        return 0;
    }
    
    int target = parse_immediate(as, operands[0]);
    uint16_t instr = (opcode_bits << 13);
    instr |= encode_unsigned(target, 13);
    
    return instr;
}

uint16_t encode_instruction(Assembler* as, const char* opcode, char operands[][MAX_LABEL_LENGTH], int num_ops, int pc) {
    char lower_opcode[MAX_LABEL_LENGTH];
    strncpy(lower_opcode, opcode, MAX_LABEL_LENGTH - 1);
    lower_opcode[MAX_LABEL_LENGTH - 1] = '\0';
    to_lowercase(lower_opcode);
    
    if (strcmp(lower_opcode, "add") == 0 || strcmp(lower_opcode, "sub") == 0 || strcmp(lower_opcode, "or") == 0 || strcmp(lower_opcode, "and") == 0 || strcmp(lower_opcode, "slt") == 0) {
        return encode_three_reg(lower_opcode, operands, num_ops);
    } 
    else if (strcmp(lower_opcode, "jr") == 0) {
        return encode_jr(operands, num_ops);
    }
    else if (strcmp(lower_opcode, "addi") == 0 || strcmp(lower_opcode, "lw") == 0 || strcmp(lower_opcode, "sw") == 0 || strcmp(lower_opcode, "jeq") == 0 || strcmp(lower_opcode, "slti") == 0) {
        return encode_two_reg(as, lower_opcode, operands, num_ops, pc);
    }
    else if (strcmp(lower_opcode, "j") == 0 || strcmp(lower_opcode, "jal") == 0) {
        return encode_jump(as, lower_opcode, operands, num_ops);
    }
    else {
        char msg[128];
        sprintf(msg, "Unknown opcode: %s", opcode);
        error(msg, current_line);
        return 0;
    }
}

void expand_pseudo(const char* opcode, char operands[][MAX_LABEL_LENGTH], int* num_ops, int pc, char* new_opcode) {
    char lower_opcode[MAX_LABEL_LENGTH];
    strncpy(lower_opcode, opcode, MAX_LABEL_LENGTH - 1);
    lower_opcode[MAX_LABEL_LENGTH - 1] = '\0';
    to_lowercase(lower_opcode);
    
    if (strcmp(lower_opcode, "movi") == 0) { // movi $reg, imm => addi $reg, $0, imm
        if (*num_ops != 2) {
            error("movi requires 2 operands", current_line);
        }
        strcpy(new_opcode, "addi");
        char temp[MAX_LABEL_LENGTH];
        strcpy(temp, operands[0]);
        strcpy(operands[0], temp);
        strcpy(operands[2], operands[1]);
        strcpy(operands[1], "$0");
        *num_ops = 3;
    }
    else if (strcmp(lower_opcode, "nop") == 0) { // nop => add $0, $0, $0
        strcpy(new_opcode, "add");
        strcpy(operands[0], "$0");
        strcpy(operands[1], "$0");
        strcpy(operands[2], "$0");
        *num_ops = 3;
    }
    else if (strcmp(lower_opcode, "halt") == 0) { // halt => j pc
        strcpy(new_opcode, "j");
        sprintf(operands[0], "%d", pc);
        *num_ops = 1;
    } else {
        strcpy(new_opcode, opcode);
    }
}

void pass1(Assembler* as, FILE* fp) {
    char line[MAX_LINE_LENGTH];
    char tokens[20][MAX_LABEL_LENGTH];
    int address = 0;
    current_line = 0;
    rewind(fp); // rewind file for pass
    
    while (fgets(line, sizeof(line), fp)) {
        current_line++;
        strip_comment(line);
        
        int num_tokens = tokenize(line, tokens, 20);
        if (num_tokens == 0) {
            continue;
        }
        
        // Extract labels
        int token_idx = 0;
        while (token_idx < num_tokens && strchr(tokens[token_idx], ':')) {
            int len = strlen(tokens[token_idx]);
            tokens[token_idx][len - 1] = '\0';
            
            if (!is_valid_label(tokens[token_idx])) {
                char msg[128];
                sprintf(msg, "Invalid label name: %s", tokens[token_idx]);
                error(msg, current_line);
            }
            
            add_symbol(as, tokens[token_idx], address);
            token_idx++;
        }
        
        if (token_idx < num_tokens) {
            if (strcmp(tokens[token_idx], ".fill") == 0) {
                address++;
            } 
            else if (tokens[token_idx][0] != '.') {
                address++;
            }
        }
    }
}

void pass2(Assembler* as, FILE* fp) {
    char line[MAX_LINE_LENGTH];
    char tokens[20][MAX_LABEL_LENGTH];
    char operands[10][MAX_LABEL_LENGTH];
    int address = 0;
    current_line = 0;
    rewind(fp); // rewind file for pass
    
    while (fgets(line, sizeof(line), fp)) {
        current_line++;
        strip_comment(line);
        int num_tokens = tokenize(line, tokens, 20);

        if (num_tokens == 0) {
            continue;
        }
        
        // Skip labels
        int token_idx = 0;
        while (token_idx < num_tokens && strchr(tokens[token_idx], ':')) {
            token_idx++;
        }
        
        if (token_idx >= num_tokens) {
            continue;
        }

        if (strcmp(tokens[token_idx], ".fill") == 0) {
            if (token_idx + 1 >= num_tokens) {
                error(".fill requires exactly 1 argument", current_line);
            }
            
            int value = parse_immediate(as, tokens[token_idx + 1]);
            if (value < 0) {
                value = (1 << 16) + value;
            }

            value = value & 0xFFFF;
            if (as->num_instructions >= MAX_INSTRUCTIONS) {
                error("Program too large (exceeds maximum instructions)", current_line);
            }

            as->machine_code[as->num_instructions++] = (uint16_t)value;
            address++;
            continue;
        }
        
        // Handle instruction
        if (tokens[token_idx][0] != '.') {
            char opcode[MAX_LABEL_LENGTH];
            strcpy(opcode, tokens[token_idx]);
            
            // Gather operands
            int num_ops = 0;
            for (int i = token_idx + 1; i < num_tokens && num_ops < 10; i++) {
                strcpy(operands[num_ops++], tokens[i]);
            }
            
            // Expand pseudo-instructions
            char expanded_opcode[MAX_LABEL_LENGTH];
            expand_pseudo(opcode, operands, &num_ops, address, expanded_opcode);
            
            uint16_t instr = encode_instruction(as, expanded_opcode, operands, num_ops, address);
            if (as->num_instructions >= MAX_INSTRUCTIONS) {
                error("Program too large (exceeds maximum instructions)", current_line);
            }
            as->machine_code[as->num_instructions++] = instr;
            address++;
        }
    }
}

void assemble(const char* input_file, const char* output_file) {
    Assembler as;
    as.num_symbols = 0;
    as.num_instructions = 0;
    
    FILE* input = fopen(input_file, "r");
    if (!input) {
        fprintf(stderr, "Error: Cannot open input file '%s'\n", input_file);
        exit(1);
    }
    
    // Pass 1: Build symbol table 
    pass1(&as, input);
    // Pass 2: Generate machine code
    pass2(&as, input);
    fclose(input);
    
    FILE* output = fopen(output_file, "w");
    if (!output) {
        fprintf(stderr, "Error: Cannot create output file '%s'\n", output_file);
        exit(1);
    }
    
    for (int i = 0; i < as.num_instructions; i++) {
        fprintf(output, "ram[%d] = 16'b", i);
        for (int bit = 15; bit >= 0; bit--) {
            fprintf(output, "%d", (as.machine_code[i] >> bit) & 1);
        }
        fprintf(output, ";\n");
    }
    
    fclose(output);
    printf("Assembly successful: %s -> %s\n", input_file, output_file);
    printf("Generated %d instructions\n", as.num_instructions);
}

char* generate_output_file(const char* inputFile) {
    const char* last_slash = strrchr(inputFile, '/');
    const char* last_backslash = strrchr(inputFile, '\\');
    const char* sep = last_slash;
    if (last_backslash && (!sep || last_backslash > sep)) {
        sep = last_backslash;
    }

    const char* filename = sep ? sep + 1 : inputFile;
    const char* dot = strrchr(filename, '.');
    size_t baseLen = dot ? (size_t)(dot - inputFile) : strlen(inputFile);
    char* output = malloc(baseLen + 5); // ".bin" + '\0'
    if (!output) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(1);
    }

    snprintf(output, baseLen + 5, "%.*s.bin", (int)baseLen, inputFile);
    return output;
}

int main(int argc, char* argv[]) {
    char* outputFile = NULL;

    if (argc == 2) {
        outputFile = generate_output_file(argv[1]);
    }
    else if (argc == 3) { // output file name provided
        outputFile = argv[2];
    }
    else {
        fprintf(stderr, "Usage: %s <input.s> [output.bin]\n", argv[0]);
        fprintf(stderr, "Must provide an intput file (.s)\n");
        fprintf(stderr, "If output.bin is not specified, it will be auto-generated\n");
        fprintf(stderr, "Example: %s input.s (produces input.bin)\n", argv[0]);
        fprintf(stderr, "         %s input.s output.bin (produces output.bin)\n", argv[0]);
        return 1;
    }
    assemble(argv[1], outputFile);

    if (argc == 2) {
        free(outputFile);
    }
}