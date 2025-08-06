#define _CRT_SECURE_NO_DEPRECATE

#include <stdint.h>
#include "my_defines.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "my_math.cpp"
#include "string.hpp"
#include "array_old.hpp"
#include "files.hpp"

void test_str1(){
    String str = init_string_from_str("Cht oto\nab{ob}a\nw|ow!!!\n");
    
    //Dynamic_Array<String> splited = split_str(str, "\n", 1);
    Dynamic_Array<Medium_Str> splitted = Dynamic_Array<Medium_Str>(16);
    
    split_str(str.data, "\n {}|", &splitted);
    
    assert(str_equal(splitted.get_value(0).data, "Cht"));
    assert(str_equal(splitted.get_value(1).data, "oto"));
    assert(str_equal(splitted.get_value(2).data, "ab"));
    assert(str_equal(splitted.get_value(3).data, "ob"));
    assert(str_equal(splitted.get_value(4).data, "a"));
    assert(str_equal(splitted.get_value(5).data, "w"));
    assert(str_equal(splitted.get_value(6).data, "ow!!!"));
    
    printf("test_str1 cool!\n");
    
    str.free_data();
    //free_string_array(&splitted);
    splitted.free_data();
}

void test_str2(){
    const char *str1 = "Ayou";
    const char *contains1 = "you";
    const char *contains2 = "u";
    const char *contains3 = "ay";
    const char *false_contains1 = "ao";
    
    const char *str2 = "block_base";
    const char *str2_contains1 = "_base";
    const char *str2_contains2 = "base";
    const char *str2_contains3 = "ase";
    const char *str2_contains4 = "se";
    
    assert(str_contains_const(str1, contains1));
    assert(str_contains_const(str1, contains2));
    assert(str_contains_const(str1, contains3));
    assert(!str_contains_const(str1, false_contains1));
    
    assert(str_contains_const(str2, str2_contains1));
    assert(str_contains_const(str2, str2_contains2));
    assert(str_contains_const(str2, str2_contains3));
    assert(str_contains_const(str2, str2_contains4));
    
    printf("test_str2 cool!\n");
}

// void test_files1(){
//     File test_file = load_file("../test_data/test_arr1.txt", "r");
    
//     int test_arr[][10] = {{123, 543, 65, 23, 54}, {534, 12, 32, 54, 5, 6, 7, 8}};
//     //int text_arr2[] = {534, 12, 32, 54, 5, 6, 7, 8};
    
//     Dynamic_Array<String> splited = Dynamic_Array<String>(16);
    
//     for (int i = 0; i < test_file.lines.count; i++){
//         splited = split_str(test_file.lines.get_value(i), "[, ]", 4);
//         for (int j = 0; j < splited.count; j++){
//             int num = atoi(splited.get_value(j).data);
//             assert(num == test_arr[i][j]);
//         }
        
//         //printf("%s", test_file.data.get_value(i).data);
//     }
    
//     printf("test_files1 cool!\n");
    
//     free_string_array(&splited);
//     unload_file(&test_file);
// }

global_variable b32 testing = true;

int main(){
    if (testing){
        test_str1();
        test_str2();
        //test_files1();
    }
}
