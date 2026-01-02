#pragma once

struct Ui_Panel {
    Rectangle rect = {0};
    
    Array <Rectangle> childs = {};
};

Array <Ui_Panel> active_panels = {};
Array <Ui_Panel> panels = {};

void begin_panel(Rectangle rect) {
    Ui_Panel panel = {0};
    
    panel.rect = rect;
    panels.append(panel, temp);
    active_panels.append(panel, temp);
}

void end_panel() { 
    assert(active_panels.count > 0);
    active_panels.pop();
}

void draw_immediate_ui() {
    // Will go from oldest panel to newest.
    for_array (i, &panels) {
        auto panel = panels.get(i);
        
        
    }
}
