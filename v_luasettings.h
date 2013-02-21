void InitLuaSettings();
void CleanupLua();
int Lua_GetIntSetting(char* varname);
double Lua_GetDoubleSetting(char* varname);
char* Lua_GetStringSetting(char* varname);