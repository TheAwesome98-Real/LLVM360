#pragma once

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//std::atomic<bool> g_continue = false;
//std::atomic<bool> g_dBUpdating = false;   
//std::vector<uint32_t> g_dBlist;

class ImGuiDebugger {
public:
    static void initStatic();

    void initialize();
    void render(); 
	void update(); 
    
    HWND debugWindow;
    //std::unordered_map<uint32_t, Instruction> dbInstrMap;
    static ImGuiDebugger* g_debugger;
    
};


