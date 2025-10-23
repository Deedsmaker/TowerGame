#pragma once

struct Ui_Panel {
    Rectangle rect = {0};
    
    Array <Rectangle> childs = {.allocator = temp};
};

Array <Ui_Panel> active_panels = {.allocator = temp};
Array <Ui_Panel> panels = {.allocator = temp};

void begin_panel(Rectangle rect) {
    Ui_Panel panel = {0};
    
    panel.rect = rect;
    panels.append(panel);
    active_panels.append(panel);
}

// void end_panel() { 
//     active_panels.pop();
// }

void draw_immediate_ui() {
    // Will go from oldest panel to newest.
    for_array(i, &panels) {
        auto panel = panels.get(i);
        
        
    }
}
