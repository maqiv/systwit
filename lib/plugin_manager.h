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


/*
 * Load the plugins in the given plugin folder and return the
 * number of loaded plugins.
 * plugin_handles is a void pointer to an array of handles pointing
 * the loaded plugins.
 */
 
 typedef struct {
   char* pname;
   char* ppath;
   void* phandle;
 } plugin_handle;
 
int plugins_count;
int files_count;
plugin_handle** plugin_handles;

int load_plugins(char* const);
void destroyPluginHandle(plugin_handle*);
plugin_handle* createNewPluginHandle(char*, char*);
char* readAbsExecPath();
char* determinePluginFolder(char*);
