#pragma once 

#include "string.cpp"
#include "memory_arena.cpp"
#include "array.cpp"

String read_entire_file(String name, b32 *success, Memory_Arena *arena = HEAP_ALLOCATOR) {
    
    char *text_data = LoadFileText(c_str(name));
    
    if (!text_data) {
        if (success) *success = false;
        return {0};
    }
    
    String s = string(arena, text_data);;
    
    UnloadFileText(text_data);    
    
    if (success) *success = true;
    
    return s;
}

b32 write_entire_file(String name, String_Builder *builder) {
    return SaveFileText(c_str(name), (char *)c_str(builder));
}

b32 append_text_to_file(String file_name, String to_append) {
    FILE *file = fopen(c_str(file_name), "a");
    if (!file) {
        return false;
    }
    
    fprintf(file, c_str(to_append));
    
    fclose(file);
    
    return true;
}
    
b32 directory_exists(String name) {
    return DirectoryExists(c_str(name));
}

b32 make_directory_if_not_exists(String name) { 
    if (!directory_exists(name)) {
        return MakeDirectory(c_str(name)) == 0;   
    }
    
    return false;
}

b32 file_exists(String name) {
    return FileExists(c_str(name));
}

b32 delete_file(String name) {
    if (!file_exists(name)) return false;
    
    return FileRemove(c_str(name));
}

// Rename file or directory.
b32 rename_file(String name, String new_name) {
    if (!file_exists(name)) return false;
    
    return FileRename(c_str(name), c_str(new_name));
}
b32 rename_directory(String name, String new_name) {
    if (!directory_exists(name)) return false;   
    
    return rename_file(name, new_name);
}

// Files or directories.
Array <String> get_files_in_directory(String directory_name, Memory_Arena *arena = HEAP_ALLOCATOR) {
    if (!directory_exists(directory_name)) return {0};
    
    FilePathList files_paths = LoadDirectoryFiles(c_str(directory_name));
    
    Array <String> result_paths = {.arena = arena};
    
    for (u32 i = 0; i < files_paths.count; i++) {
        result_paths.append(string(arena, files_paths.paths[i]));
    }
    
    UnloadDirectoryFiles(files_paths);
    
    return result_paths;
}

// Will replace string level/file.txt to just file.txt (detects / or \\).
String strip_path_to_just_name(String path, Memory_Arena *arena) {
    i32 slash_index = string_find_from_back(path, S("/"));
    i32 backslash_index = string_find_from_back(path, S("\\"));
    
    i32 index_from = (slash_index > backslash_index ? slash_index : backslash_index) + 1; // +1 so that we'll start form real name.
    
    return make_substring(path, index_from, path.count - 1, arena);
}

String remove_extension(String name, Memory_Arena *arena) {
    i32 dot_index = string_find_from_back(name, S("."));
    if (dot_index <= 0) return {0};
    
    String s = make_substring(name, 0, dot_index - 1, arena);
    return s;
}

u64 get_file_modification_time(String path) {
    return GetFileModTime(c_str(path));
}

inline i32 find_file_name_in_paths(Array <String> *paths, String name_to_find, Memory_Arena *arena) {
    for_array (i, paths) {
        String name = strip_path_to_just_name(paths->get_value(i), temp);
        if (name == name_to_find) return i;
    }
    
    return -1;
}

Array <String> get_file_names_in_directory(String directory_name, Memory_Arena *arena) {
    if (!directory_exists(directory_name)) return {0};
    
    FilePathList files_paths = LoadDirectoryFiles(c_str(directory_name));
    
    Array <String> result_paths = {.arena = arena};
    
    for (u32 i = 0; i < files_paths.count; i++) {
        String full_path = string(temp, files_paths.paths[i]);
        String name = strip_path_to_just_name(full_path, arena);
    
        result_paths.append(name);
    }
    
    UnloadDirectoryFiles(files_paths);
    
    return result_paths;
}

#ifdef _WIN32
    #include <direct.h>
#endif

b32 delete_directory(String path) {
    if (!directory_exists(path)) return false;
      
    auto files = get_files_in_directory(path, temp);
    
    for_array (i, &files) {
        delete_file(files.get_value(i));
    }
    
    return _rmdir(c_str(path)) == 0;
}

// This compare functions differs form just lexegraphical compare because we'll detect number in string and will consider is 
// as a main compare point. We're need this because we want to load entities in right id order (that will not 
// be required eventually, but we need it right now).
i32 file_paths_compare(String s1, String s2) {
    String name1 = strip_path_to_just_name(s1, temp);
    String name2 = strip_path_to_just_name(s2, temp);
    
    i32 digit_index1 = string_find_digit(name1);
    i32 digit_index2 = string_find_digit(name2);
    
    // If only one of them have digit in it - we count one without digit to be earlier after sorting.
    // That's just an arbitrary decision.
    if (digit_index1 < 0 && digit_index2 >= 0) return -1;
    if (digit_index2 < 0 && digit_index1 >= 0) return 1;
    
    // If digit was found - we're sorting considering it. If not - sorting just lexagraphically.
    if (digit_index1 >= 0 && digit_index2 >= 0) {
        i32 first_number  = to_i32(make_substring(name1, digit_index1, name1.count - 1, temp));
        i32 second_number = to_i32(make_substring(name2, digit_index2, name2.count - 1, temp));
        
        if (first_number < second_number) return -1;
        if (second_number - first_number) return 1;
    }
    
    // Simple strcmp style comparing.
    
    i32 index = 0;
    while (name1.count > index && name2.count > index && name1.data[index] == name2.data[index]) {
        index += 1;
    }
    
    return name2.count - name1.count;
}

inline i32 file_paths_compare(const void *s1, const void *s2) {
    return file_paths_compare(*(String *)s1, *(String *)s2/*, true*/);
}

void sort_file_paths(Array <String> *array) {
    qsort(array->data, array->count, sizeof(String), file_paths_compare);
}

