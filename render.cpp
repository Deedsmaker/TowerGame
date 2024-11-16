#pragma once

void draw_texture(Texture tex, Vector2 pos, Color tint){
    DrawTextureV(tex, pos, tint);
}

void draw_texture(Texture tex, int x, int y, Color tint){
    DrawTexture(tex, x, y, tint);
}

void draw_texture(Texture tex, Vector2 pos, Vector2 scale, Vector2 pivot, float rotation, Color tint){
    Vector2 target_size = {tex.width * scale.x, tex.height * scale.y};
    
    Vector2 origin = {target_size.x * pivot.x, target_size.y * pivot.y};
    Rectangle source = {0.0f, 0.0f, (f32)tex.width, (f32)tex.height};
    Rectangle dest = {pos.x, pos.y, (f32)tex.width * scale.x, (f32)tex.height * scale.y};
    
    DrawTexturePro(tex, source, dest, origin, rotation, tint);
}

void draw_rect(Vector2 pos, Vector2 size, Color color){
    DrawRectangleV(pos, size, color);
}
void draw_rect(Vector2 pos, Vector2 size, f32 rotation, Color color){
    DrawRectanglePro({pos.x, pos.y, size.x, size.y}, {size.x * 0.5f, size.y * 0.5f}, rotation, color);
}
void draw_rect(Vector2 pos, Vector2 size, Vector2 pivot, f32 rotation, Color color){
    DrawRectanglePro({pos.x, pos.y, size.x, size.y}, {size.x * pivot.x, size.y * pivot.y}, rotation, color);
}
void draw_rect(int x, int y, int width, int height, Color color){
    DrawRectangle(x, y, width, height, color);
}

void draw_rect_lines(Vector2 pos, Vector2 scale, f32 thick, Color color){
    Rectangle rec = {pos.x, pos.y, scale.x, scale.y};
    DrawRectangleLinesEx(rec, fmax(3, thick), color);
}

void draw_line_strip(Vector2 *points, int count, Color color){
    DrawLineStrip(points, count, color);
}

void draw_triangle_strip(Vector2 *points, int count, Color color){
    DrawTriangleStrip(points, count, color);
}

void draw_circle(Vector2 pos, f32 radius, Color color){
    DrawCircle(pos.x, pos.y, radius, color);
}

void draw_line(Vector2 start, Vector2 end, f32 thick, Color color){
    DrawLineEx(start, end, fmax(3, thick), color);
}

void draw_text(const char *text, Vector2 pos, float size, Color color){
    DrawText(text, pos.x, pos.y, size, color);
}

void draw_text(const char *text, float x, float y, float size, Color color){
    DrawText(text, x, y, size, color);
}

void draw_text(int num, float x, float y, float size, Color color){
    char *str = to_string(num);
    DrawText(str, x, y, size, color);
    free(str);
}

void draw_text(f32 num, float x, float y, float size, Color color){
    char *str = to_string(num);
    DrawText(str, x, y, size, color);
    free(str);
}

void draw_text(f64 num, float x, float y, float size, Color color){
    char *str = to_string(num);
    DrawText(str, x, y, size, color);
    free(str);
}

void draw_text(int num, Vector2 pos, float size, Color color){
    char *str = to_string(num);
    DrawText(str, pos.x, pos.y, size, color);
    free(str);
}
void draw_text(f32 num, Vector2 pos, float size, Color color){
    char *str = to_string(num);
    DrawText(str, pos.x, pos.y, size, color);
    free(str);
}
void draw_text(f64 num, Vector2 pos, float size, Color color){
    char *str = to_string(num);
    DrawText(str, pos.x, pos.y, size, color);
    free(str);
}

int get_shader_location(Shader shader, const char *name){
    return GetShaderLocation(shader, name);
}

void set_shader_value(Shader shader, int loc, float value){
    SetShaderValue(shader, loc, &value, SHADER_UNIFORM_FLOAT);
}

void set_shader_texture(Shader shader, int loc, Texture tex){
    SetShaderValueTexture(shader, loc, tex);
}
    
void draw_text_boxed(const char *text, Rectangle rec, float fontSize, float spacing, Color tint)
{
    int length = str_len(text); 

    float textOffsetY = 0;        
    float textOffsetX = 0.0f;     

    Font font = GetFontDefault();
    float scaleFactor = fontSize/(float)font.baseSize; 

    // Word/character wrapping mechanism variables
    int startLine = -1;         // Index where to begin drawing (where a line begins)
    int endLine = -1;           // Index where to stop drawing (where a line ends)
    int lastk = -1;             // Holds last value of the character position

    for (int i = 0; i < length; i++){
        if (text[i] == '\n'){
            textOffsetY += font.baseSize * scaleFactor;
        }
    }
    textOffsetY -= font.baseSize * scaleFactor;

    for (int i = 0; i < length; i++)
    {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetCodepoint(&text[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        // NOTE: Normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol moving one byte
        if (codepoint == 0x3f) codepointByteCount = 1;
        i += (codepointByteCount - 1);

        float glyphWidth = 0;
        if (codepoint != '\n')
        {
            glyphWidth = (font.glyphs[index].advanceX == 0) ? font.recs[index].width*scaleFactor : font.glyphs[index].advanceX*scaleFactor;

            if (i + 1 < length) glyphWidth = glyphWidth + spacing;
        }
       
        if (codepoint == '\n')
        {
            textOffsetY -= (font.baseSize)*scaleFactor;
            textOffsetX = 0;
        }
        else
        {
            if ((textOffsetX + glyphWidth) > rec.width)
            {
                textOffsetY += (font.baseSize)*scaleFactor;
                textOffsetX = 0;
            }

            // When text overflows rectangle height limit, just stop drawing
            if ((textOffsetY + font.baseSize*scaleFactor) > rec.height) break;

            // Draw current character glyph
            if ((codepoint != ' ') && (codepoint != '\t'))
            {
                DrawTextCodepoint(font, codepoint, (Vector2){ rec.x + textOffsetX, rec.y + rec.height - textOffsetY }, fontSize, tint);
            }
        }

        if ((textOffsetX != 0) || (codepoint != ' ')) textOffsetX += glyphWidth;  // avoid leading spaces
    }
}




