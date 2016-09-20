/*
 * Library to load dynamic library objects. 
 *
 * Copyright (C) 2015-2016 Sebastian Glinski <sebastian.glinski@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <dirent.h>
#include <dlfcn.h>
#include <sys/param.h>
#include "plugin_manager.h"

/*
 * Load plugins (shared object files *.so) from the given directory
 * configured in PLUGIN_FOLDER.
 */
int load_plugins(char* const p_folder) {
  char* file_ending;
  struct dirent *entry;

  char* plugin_folder = determinePluginFolder(p_folder);
  files_count = count_files_in_dir(plugin_folder);
  DIR* dir = opendir(plugin_folder);
  plugins_count = 0;
  
  if (!plugin_handles)
    free(plugin_handles);
  
  if ((plugin_handles = malloc(sizeof(plugin_handle) * files_count)) != NULL) {
    while ((entry = readdir(dir)) != NULL) {
      
      /* Check if a file has the correct ending for a dynamic library */
      file_ending = strrchr(entry->d_name, '.');
      if ((!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) &&
          !(file_ending && !strcmp(file_ending, ".so")))
        continue;

      plugin_handle* ph = createNewPluginHandle(plugin_folder, entry->d_name);
      /* printf("%s\n", ph->ppath); */
      ph->phandle = dlopen(ph->ppath, RTLD_NOW);
      printf("%s\n", ph->ppath);
      char* err = dlerror();
      if (err) {
        printf("error: %s\n", err);
      }
      
      /* Add the loaded plugin to the collection */
      plugin_handles[plugins_count] = ph;
      
      plugins_count++;
    }
  }
  
  return plugins_count;
}

/* 
 * Create a new plugin handle and gather
 * information about the plugin to load.
 */
plugin_handle* createNewPluginHandle(char* plugin_folder, char* d_name) {
  plugin_handle* ph = malloc(sizeof(plugin_handle));
  int pfLen = strlen(plugin_folder);
  int peLen = strlen(d_name);

  //ph = (plugin_handle*)malloc(sizeof(plugin_handle));
  ph->ppath = malloc(sizeof(char) * (pfLen + peLen + 2)); /* +2 because of the / and the \0 */
  ph->pname = malloc(sizeof(char) * peLen);

  strcpy(ph->pname, d_name);
  sprintf(ph->ppath, "%s/%s", plugin_folder, d_name);

  return ph;
}

/*
 * Free up the ressources/memory of the given
 * plugin handle.
 */
void destroyPluginHandle(plugin_handle* ph) {
  free(ph->pname);
  free(ph->ppath);
  free(ph->phandle);
  free(ph);
  ph = NULL;
}

/*
 * Count the files in the plugins directory so that
 * we know the size of the of the array we need to
 * allocate with void pointers.
 */
int count_files_in_dir(char* plugin_folder) {
  int files_count = 0;
  struct dirent* e;
  char* file_ending;
  DIR* dir = opendir(plugin_folder);
  
  while ((e = readdir(dir)) != NULL) {
    
    /* Skip unnecessary entries from readdir() */
    if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, ".."))
      continue;
    
    /* Check if a file has the correct ending for a dynamic library */
    file_ending = strrchr(e->d_name, '.');
    if (file_ending && !strcmp(file_ending, ".so")) {
      files_count++;
    }
  }
  
  if (dir != NULL) {
    closedir(dir);
  }
  
  return files_count;
}

void execute_plugins(char* content) {
  int i;
  char* err;
  void (*handle) (char*);
  plugin_handle* p_handle;
  
  /* Clear the last error code if there is one */
  dlerror();
  
  for (i = 0; i < plugins_count; i++) {
    
    /* Load the plugin information from our plugin pool */
    p_handle = plugin_handles[i];
    
    /* Execute the function in the plugin */
    handle = dlsym(p_handle->phandle, "write_content");
    handle(content);
    /* Check if there occured an error */
    if ((err = dlerror()) != NULL) {
      syslog(LOG_ERR, "systwit - error (%s): %s", p_handle->pname, err);
    } /* else {
      syslog(LOG_INFO, "systwit - successfully executed plugin %s", p_handle->pname);
    }*/

    /*
     * TODO: Free close and free up plugin handles
     *
    if(dlclose(handle) > 0) {
      syslog(LOG_ERR, "could not unload plugin: %s", p_handle->pname);
    }

    destroyPluginHandle(p_handle);
    */
  }
}

char* determinePluginFolder(char* p_folder) {
  char* absExecPath = readAbsExecPath();
  int pfLen = strlen(p_folder);
  int peLen = strlen(absExecPath);

  char* folder = (char*)malloc(sizeof(char) * (pfLen + peLen + 1));
  strcpy(folder, absExecPath);
  strcat(folder, p_folder);

  free(absExecPath);

  return folder;
}

char* readAbsExecPath() {

  /*
   * Resolve the relative path where the running binary
   * is located in.
   */
  char* absExecPath = malloc(sizeof(char) * PATH_MAX);
  realpath("/proc/self/exe", absExecPath);

  /*
   * Trim down the pathname from the tail to remove the
   * executables name itself.
   */
  int pLen = strlen(absExecPath);
  int i;
  for (i = (pLen - 1); pLen > i; i--) {
    if (absExecPath[i] == '/') {
      break;
    }
    absExecPath[i] = '\0';
  }

  return absExecPath;
}
/* Unload all plugins with dlclose() */
