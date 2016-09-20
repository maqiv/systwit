#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>

int load_plugins(char const plugin_folder[], void* plugin_handles) {
  int plugins_count = 0;
  DIR *dir = opendir(plugin_folder);
  struct dirent *entry;
  char *file_ending;
  char *blamuh;
  
  while((entry = readdir(dir)) != NULL) {
    if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
      continue;
    
    printf("%s %zu\n", entry->d_name, strlen(entry->d_name)); 
    
    if (!blamuh)
      free(blamuh);
    
    file_ending = strrchr(entry->d_name, '.');
    if(file_ending && !strcmp(file_ending, ".so")) {
      blamuh = malloc(strlen(plugin_folder) + 2 + strlen(entry->d_name));
      sprintf(blamuh, "%s/%s", plugin_folder, entry->d_name);
      printf("found valid file: %s", blamuh);
      /*free(blamuh);*/
      plugins_count++;
    }
    
    plugins_count++;
  }
  
  return plugins_count;
}

int main(int argc, char* argv[]) {
  void* my_handles;
  int numb = load_plugins("./bla", my_handles);
  
  printf("%i\n", numb);
  
  return 0;
}

