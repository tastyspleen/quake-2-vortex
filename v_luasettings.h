void InitLuaSettings();
void CleanupLua();
int Lua_GetIntSetting(char* varname);
double Lua_GetDoubleSetting(char* varname);
const char* Lua_GetStringSetting(char* varname);
void RunLuaMapSettings(char* mapname);
double Lua_GetVariable(char* varname, double default_var);