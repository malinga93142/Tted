#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

typedef struct{
  unsigned start, end;
} lines;

typedef enum {
  CMD_LINE,
  CMD_LINES,
  CMD_DELETE,
  CMD_APPEND,
  CMD_REPLACE
} command_type;

typedef enum {
  MODE_TERMINAL,  // Display only, no file modification
  MODE_INPLACE    // Modify files in-place
} exec_mode;

lines parse_command(const char *str, bool multi_lines){
  char buffer[9];
  lines line;
  int k = 0;
  while (*str == 0x20)  str++;
  if (multi_lines){
    while (*str){
      if (isdigit(*str)){
        buffer[k++] = *str++;
      } else  break;
    }
    buffer[k] = '\0';
    line.start = atoi(buffer);
    k = 0;
    memset(buffer, 0, sizeof(buffer));
    while (*str == 0x20 || *str == ',') str++;
    while (*str){
      if (isdigit(*str)){
        buffer[k++] = *str++;
      } else break;
    }
    buffer[k] = '\0';
    line.end = (unsigned)atoi(buffer);
  } else{
    while (*str){
      if (isdigit(*str)){
        buffer[k++] = *str++;
      } else  break;
    }
    buffer[k] = '\0';
    line.start = (unsigned)atoi(buffer);
    line.end = line.start;
  }
  return line;
}

unsigned total_lines(const char *path){
  if (path == NULL){
    return 0;
  }
  FILE *fptr = fopen(path, "r");
  if (!fptr)  return 0;
  int count = 1;
  char buf[512];
  while (fgets(buf, sizeof(buf), fptr) != NULL) count++;
  fclose(fptr);
  return count;
}

void read_lines(const char *filepath, lines line, bool multi_lines){
  FILE *fptr = fopen(filepath, "r");
  if (!fptr) return;
  
  char line_buf[512];
  unsigned current_line = 1;
  if (multi_lines){
    while (fgets(line_buf, sizeof(line_buf), fptr) != NULL){
      if (current_line >= line.start && current_line <= line.end){
        printf("%3d:%s", current_line, line_buf);
      }
      if (++current_line > line.end)  break;
    }
  } else{
    while (fgets(line_buf, sizeof(line_buf), fptr) != NULL){
      if (current_line == line.start){
        printf("%s", line_buf);
        break;
      }
      current_line++;
    }
  }
  fclose(fptr);
}

void delete_lines(const char *filepath, lines line, exec_mode mode){
  FILE *fptr = fopen(filepath, "r");
  if (!fptr) {
    fprintf(stderr, "Error: Cannot open file\n");
    return;
  }
  
  int __lines = total_lines(filepath);
  if (line.start > __lines || line.end > __lines || line.start > line.end || line.start == 0){
    fclose(fptr);
    fprintf(stderr, "Invalid range: Operation is not permitted\n");
    return;
  }
  
  if (mode == MODE_TERMINAL){
    // Terminal mode: display result without modifying file
    char line_buf[512];
    unsigned current_line = 1;
//    printf("--- Preview (lines %u-%u deleted) ---\n", line.start, line.end);
    while (fgets(line_buf, sizeof(line_buf), fptr) != NULL){
      if (current_line < line.start || current_line > line.end){
        printf("%3d:%s", current_line, line_buf);
      }
      current_line++;
    }
    fclose(fptr);
    return;
  }
  
  // In-place mode: modify the file
  FILE *temp = fopen("temp_file.tmp", "w");
  if (!temp) {
    fprintf(stderr, "Error: Cannot create temporary file\n");
    fclose(fptr);
    return;
  }
  
  char line_buf[512];
  unsigned current_line = 1;
  
  while (fgets(line_buf, sizeof(line_buf), fptr) != NULL){
    if (current_line < line.start || current_line > line.end){
      fputs(line_buf, temp);
    }
    current_line++;
  }
  
  fclose(fptr);
  fclose(temp);
  
  remove(filepath);
  rename("temp_file.tmp", filepath);
  printf("Deleted lines %u to %u from %s\n", line.start, line.end, filepath);
}

void append_text(const char *filepath, unsigned line_num, const char *text, exec_mode mode){
  FILE *fptr = fopen(filepath, "r");
  if (!fptr) {
    fprintf(stderr, "Error: Cannot open file\n");
    return;
  }
  
  int __lines = total_lines(filepath);
  if (line_num > __lines || line_num == 0){
    fclose(fptr);
    fprintf(stderr, "Invalid range: Operation is not permitted\n");
    return;
  }
  
  if (mode == MODE_TERMINAL){
    // Terminal mode: display result without modifying file
    char line_buf[512];
    unsigned current_line = 1;
   // printf("--- Preview (text appended after line %u) ---\n", line_num);
    while (fgets(line_buf, sizeof(line_buf), fptr) != NULL){
      if (current_line == line_num){
        printf("%3d:\x1b[1;31m%s\x1b[0m\n", current_line, text);
      }
			else {
				printf("%3d:%s", current_line, line_buf);
			}
      current_line++;
    }
    fclose(fptr);
    return;
  }
  
  // In-place mode: modify the file
  FILE *temp = fopen("temp_file.tmp", "w");
  if (!temp) {
    fprintf(stderr, "Error: Cannot create temporary file\n");
    fclose(fptr);
    return;
  }
  
  char line_buf[512];
  unsigned current_line = 1;
  
  while (fgets(line_buf, sizeof(line_buf), fptr) != NULL){
    fputs(line_buf, temp);
    if (current_line == line_num){
      fprintf(temp, "%s\n", text);
    }
    current_line++;
  }
  
  fclose(fptr);
  fclose(temp);
  
  remove(filepath);
  rename("temp_file.tmp", filepath);
  printf("Appended text after line %u in %s\n", line_num, filepath);
}

void replace_text(const char *filepath, const char *old_text, const char *new_text, exec_mode mode){
  FILE *fptr = fopen(filepath, "r");
  if (!fptr) {
    fprintf(stderr, "Error: Cannot open file\n");
    return;
  }
  
  if (mode == MODE_TERMINAL){
    // Terminal mode: display result without modifying file
    char line_buf[512];
    int replacements = 0;
    unsigned current_line = 1;
    printf("--- Preview (replacing \"%s\" with \"%s\") ---\n", old_text, new_text);
    
    while (fgets(line_buf, sizeof(line_buf), fptr) != NULL){
      char *pos = strstr(line_buf, old_text);
      if (pos != NULL){
        replacements++;
        printf("%3d:", current_line);
        // Display everything before the match
        fwrite(line_buf, 1, pos - line_buf, stdout);
        // Display the new text
        fputs(new_text, stdout);
        // Display everything after the match
        fputs(pos + strlen(old_text), stdout);
      } else {
        printf("%3d:%s", current_line, line_buf);
      }
      current_line++;
    }
    fclose(fptr);
    printf("--- Would replace %d occurrence(s) ---\n", replacements);
    return;
  }
  
  // In-place mode: modify the file
  FILE *temp = fopen("temp_file.tmp", "w");
  if (!temp) {
    fprintf(stderr, "Error: Cannot create temporary file\n");
    fclose(fptr);
    return;
  }
  
  char line_buf[512];
  int replacements = 0;
  
  while (fgets(line_buf, sizeof(line_buf), fptr) != NULL){
    char *pos = strstr(line_buf, old_text);
    if (pos != NULL){
      replacements++;
      fwrite(line_buf, 1, pos - line_buf, temp);
      fputs(new_text, temp);
      fputs(pos + strlen(old_text), temp);
    } else {
      fputs(line_buf, temp);
    }
  }
  
  fclose(fptr);
  fclose(temp);
  
  remove(filepath);
  rename("temp_file.tmp", filepath);
  printf("Replaced %d occurrence(s) of \"%s\" with \"%s\" in %s\n", 
         replacements, old_text, new_text, filepath);
}

void file_handle(int argc, char **argv, exec_mode mode){
  lines line;
  command_type cmd;
  int arg_offset = (mode == MODE_INPLACE) ? 1 : 0;  // Adjust for -i flag
  
  if (strcmp(argv[1 + arg_offset], "--line") == 0){
    cmd = CMD_LINE;
    line = parse_command(argv[2 + arg_offset], false);
    unsigned __lines = total_lines(argv[3 + arg_offset]);
    if (__lines == 0 || line.start > __lines){
      fprintf(stderr, "Invalid line number\n");
      return;
    }
    read_lines(argv[3 + arg_offset], line, false);
  } 
  else if (strcmp(argv[1 + arg_offset], "--lines") == 0){
    cmd = CMD_LINES;
    line = parse_command(argv[2 + arg_offset], true);
    unsigned __lines = total_lines(argv[3 + arg_offset]);
    if (__lines == 0 || line.start > __lines || line.start > line.end || line.end > __lines){
      fprintf(stderr, "Invalid line range\n");
      return;
    }
    read_lines(argv[3 + arg_offset], line, true);
  }
  else if (strcmp(argv[1 + arg_offset], "--delete") == 0){
    cmd = CMD_DELETE;
    line = parse_command(argv[2 + arg_offset], true);
    unsigned __lines = total_lines(argv[3 + arg_offset]);
    if (__lines == 0 || line.start > __lines || line.start > line.end || line.end > __lines){
      fprintf(stderr, "Invalid line range\n");
      return;
    }
    delete_lines(argv[3 + arg_offset], line, mode);
  }
  else if (strcmp(argv[1 + arg_offset], "--append") == 0){
    cmd = CMD_APPEND;
    line = parse_command(argv[2 + arg_offset], false);
    unsigned __lines = total_lines(argv[3 + arg_offset]);
    if (__lines == 0 || line.start > __lines){
      fprintf(stderr, "Invalid line number\n");
      return;
    }
    if (argc < 5 + arg_offset){
      fprintf(stderr, "Error: Text to append is required\n");
      return;
    }
    append_text(argv[3 + arg_offset], line.start, argv[4 + arg_offset], mode);
  }
  else if (strcmp(argv[1 + arg_offset], "--replace") == 0){
    cmd = CMD_REPLACE;
    if (argc < 4 + arg_offset){
      fprintf(stderr, "Error: Replace requires old/new text and filename\n");
      return;
    }
    char *slash = strchr(argv[2 + arg_offset], '/');
    if (!slash){
      fprintf(stderr, "Error: Replace format should be \"OldText/NewText\"\n");
      return;
    }
    *slash = '\0';
    char *old_text = argv[2 + arg_offset];
    char *new_text = slash + 1;
    replace_text(argv[3 + arg_offset], old_text, new_text, mode);
  }
  else {
    fprintf(stderr, "Unknown command: %s\n", argv[1 + arg_offset]);
  }
}

int main(int argc, char *argv[]){
  exec_mode mode = MODE_TERMINAL;  // Default to terminal mode
  int start_arg = 1;
  
  // Check for -i flag
  if (argc > 1 && strcmp(argv[1], "-i") == 0){
    mode = MODE_INPLACE;
    start_arg = 2;
  }
  
  if (argc < start_arg + 3){
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  Terminal mode (preview only):\n");
    fprintf(stderr, "    %s --line [#line] [file]\n", argv[0]);
    fprintf(stderr, "    %s --lines [#start,#end] [file]\n", argv[0]);
    fprintf(stderr, "    %s --delete [#start,#end] [file]\n", argv[0]);
    fprintf(stderr, "    %s --append [#line] [file] \"text\"\n", argv[0]);
    fprintf(stderr, "    %s --replace \"old/new\" [file]\n", argv[0]);
    fprintf(stderr, "\n");
    fprintf(stderr, "  In-place edit mode (modifies file):\n");
    fprintf(stderr, "    %s -i --delete [#start,#end] [file]\n", argv[0]);
    fprintf(stderr, "    %s -i --append [#line] [file] \"text\"\n", argv[0]);
    fprintf(stderr, "    %s -i --replace \"old/new\" [file]\n", argv[0]);
    return 1;
  }
  
  file_handle(argc, argv, mode);
  return 0;
}
