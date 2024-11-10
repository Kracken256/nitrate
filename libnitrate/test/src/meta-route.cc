#include <gtest/gtest.h>
#include <nitrate/code.h>

#include <cstdint>
#include <string_view>

static std::string_view source_code = R"(fn main(args: [str]): i32 {
  // Check if the user provided a file
  retif !args, print("Usage: test <file>\n"), 1;

  /**
    * Open the file and read its content
    * 
    * The file is closed automatically when the variable goes out of scope
    */
  let file = std::open(args[0], "r");
  let content = file.read_all();
  file.close();

  let x = @(nit.emit("1e43")) - 0b1001010010;
  print(x, content.as_bytes().transmute("enHEX").join(":").as_str());

  ret 0;
}
)";

static std::string_view expected_json =
    R"ESCAPE([[3,"fn",1,1,1,3],[6,"main",1,4,1,8],[5,"(",1,8,1,9],[6,"args",1,9,1,14],[5,":",1,14,1,14],[5,"[",1,15,1,16],[6,"str",1,16,1,19],[5,"]",1,19,1,20],[5,")",1,20,1,21],[5,":",1,21,1,22],[6,"i32",1,23,1,26],[5,"{",1,27,1,28],[13," Check if the user provided a file",2,3,2,39],[3,"retif",3,3,3,8],[4,"!",3,9,3,11],[6,"args",3,11,3,14],[5,",",3,14,3,15],[6,"print",3,16,3,21],[5,"(",3,21,3,22],[9,"Usage: test <file>\n",3,22,3,44],[5,")",3,44,3,45],[5,",",3,45,3,46],[7,"1",3,47,3,48],[5,";",3,48,3,49],[13,"*\n    * Open the file and read its content\n    * \n    * The file is closed automatically when the variable goes out of scope\n    ",5,3,9,6],[3,"let",10,3,10,6],[6,"file",10,7,10,11],[4,"=",10,12,10,14],[6,"std::open",10,14,10,23],[5,"(",10,23,10,24],[6,"args",10,24,10,28],[5,"[",10,28,10,29],[7,"0",10,29,10,30],[5,"]",10,30,10,31],[5,",",10,31,10,32],[9,"r",10,33,10,36],[5,")",10,36,10,37],[5,";",10,37,10,38],[3,"let",11,3,11,6],[6,"content",11,7,11,14],[4,"=",11,15,11,17],[6,"file",11,17,11,21],[4,".",11,21,11,23],[6,"read_all",11,23,11,30],[5,"(",11,30,11,31],[5,")",11,31,11,32],[5,";",11,32,11,33],[6,"file",12,3,12,7],[4,".",12,7,12,9],[6,"close",12,9,12,13],[5,"(",12,13,12,14],[5,")",12,14,12,15],[5,";",12,15,12,16],[3,"let",14,3,14,6],[6,"x",14,7,14,8],[4,"=",14,9,14,11],[8,"10000000000000000139372116959414099130712064.00000000000000000000",1,1,1,3],[4,"-",14,31,14,33],[7,"594",14,33,14,45],[5,";",14,45,14,46],[6,"print",15,3,15,8],[5,"(",15,8,15,9],[6,"x",15,9,15,10],[5,",",15,10,15,11],[6,"content",15,12,15,19],[4,".",15,19,15,21],[6,"as_bytes",15,21,15,28],[5,"(",15,28,15,29],[5,")",15,29,15,30],[4,".",15,30,15,32],[6,"transmute",15,32,15,40],[5,"(",15,40,15,41],[9,"enHEX",15,41,15,48],[5,")",15,48,15,49],[4,".",15,49,15,51],[6,"join",15,51,15,54],[5,"(",15,54,15,55],[9,":",15,55,15,58],[5,")",15,58,15,59],[4,".",15,59,15,61],[6,"as_str",15,61,15,66],[5,"(",15,66,15,67],[5,")",15,67,15,68],[5,")",15,68,15,69],[5,";",15,69,15,70],[3,"ret",17,3,17,6],[7,"0",17,7,17,8],[5,";",17,8,17,9],[5,"}",18,1,18,2],[1,"",0,0,0,0]])ESCAPE";

static const uint8_t expected_msgpack[] = {
    0xdd, 0x00, 0x00, 0x00, 0x5a, 0x96, 0x03, 0xa2, 0x66, 0x6e, 0x01, 0x01, 0x01, 0x03, 0x96, 0x06,
    0xa4, 0x6d, 0x61, 0x69, 0x6e, 0x01, 0x04, 0x01, 0x08, 0x96, 0x05, 0xa1, 0x28, 0x01, 0x08, 0x01,
    0x09, 0x96, 0x06, 0xa4, 0x61, 0x72, 0x67, 0x73, 0x01, 0x09, 0x01, 0x0e, 0x96, 0x05, 0xa1, 0x3a,
    0x01, 0x0e, 0x01, 0x0e, 0x96, 0x05, 0xa1, 0x5b, 0x01, 0x0f, 0x01, 0x10, 0x96, 0x06, 0xa3, 0x73,
    0x74, 0x72, 0x01, 0x10, 0x01, 0x13, 0x96, 0x05, 0xa1, 0x5d, 0x01, 0x13, 0x01, 0x14, 0x96, 0x05,
    0xa1, 0x29, 0x01, 0x14, 0x01, 0x15, 0x96, 0x05, 0xa1, 0x3a, 0x01, 0x15, 0x01, 0x16, 0x96, 0x06,
    0xa3, 0x69, 0x33, 0x32, 0x01, 0x17, 0x01, 0x1a, 0x96, 0x05, 0xa1, 0x7b, 0x01, 0x1b, 0x01, 0x1c,
    0x96, 0x0d, 0xd9, 0x22, 0x20, 0x43, 0x68, 0x65, 0x63, 0x6b, 0x20, 0x69, 0x66, 0x20, 0x74, 0x68,
    0x65, 0x20, 0x75, 0x73, 0x65, 0x72, 0x20, 0x70, 0x72, 0x6f, 0x76, 0x69, 0x64, 0x65, 0x64, 0x20,
    0x61, 0x20, 0x66, 0x69, 0x6c, 0x65, 0x02, 0x03, 0x02, 0x27, 0x96, 0x03, 0xa5, 0x72, 0x65, 0x74,
    0x69, 0x66, 0x03, 0x03, 0x03, 0x08, 0x96, 0x04, 0xa1, 0x21, 0x03, 0x09, 0x03, 0x0b, 0x96, 0x06,
    0xa4, 0x61, 0x72, 0x67, 0x73, 0x03, 0x0b, 0x03, 0x0e, 0x96, 0x05, 0xa1, 0x2c, 0x03, 0x0e, 0x03,
    0x0f, 0x96, 0x06, 0xa5, 0x70, 0x72, 0x69, 0x6e, 0x74, 0x03, 0x10, 0x03, 0x15, 0x96, 0x05, 0xa1,
    0x28, 0x03, 0x15, 0x03, 0x16, 0x96, 0x09, 0xb3, 0x55, 0x73, 0x61, 0x67, 0x65, 0x3a, 0x20, 0x74,
    0x65, 0x73, 0x74, 0x20, 0x3c, 0x66, 0x69, 0x6c, 0x65, 0x3e, 0x0a, 0x03, 0x16, 0x03, 0x2c, 0x96,
    0x05, 0xa1, 0x29, 0x03, 0x2c, 0x03, 0x2d, 0x96, 0x05, 0xa1, 0x2c, 0x03, 0x2d, 0x03, 0x2e, 0x96,
    0x07, 0xa1, 0x31, 0x03, 0x2f, 0x03, 0x30, 0x96, 0x05, 0xa1, 0x3b, 0x03, 0x30, 0x03, 0x31, 0x96,
    0x0d, 0xd9, 0x81, 0x2a, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x2a, 0x20, 0x4f, 0x70, 0x65, 0x6e, 0x20,
    0x74, 0x68, 0x65, 0x20, 0x66, 0x69, 0x6c, 0x65, 0x20, 0x61, 0x6e, 0x64, 0x20, 0x72, 0x65, 0x61,
    0x64, 0x20, 0x69, 0x74, 0x73, 0x20, 0x63, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x0a, 0x20, 0x20,
    0x20, 0x20, 0x2a, 0x20, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x2a, 0x20, 0x54, 0x68, 0x65, 0x20, 0x66,
    0x69, 0x6c, 0x65, 0x20, 0x69, 0x73, 0x20, 0x63, 0x6c, 0x6f, 0x73, 0x65, 0x64, 0x20, 0x61, 0x75,
    0x74, 0x6f, 0x6d, 0x61, 0x74, 0x69, 0x63, 0x61, 0x6c, 0x6c, 0x79, 0x20, 0x77, 0x68, 0x65, 0x6e,
    0x20, 0x74, 0x68, 0x65, 0x20, 0x76, 0x61, 0x72, 0x69, 0x61, 0x62, 0x6c, 0x65, 0x20, 0x67, 0x6f,
    0x65, 0x73, 0x20, 0x6f, 0x75, 0x74, 0x20, 0x6f, 0x66, 0x20, 0x73, 0x63, 0x6f, 0x70, 0x65, 0x0a,
    0x20, 0x20, 0x20, 0x20, 0x05, 0x03, 0x09, 0x06, 0x96, 0x03, 0xa3, 0x6c, 0x65, 0x74, 0x0a, 0x03,
    0x0a, 0x06, 0x96, 0x06, 0xa4, 0x66, 0x69, 0x6c, 0x65, 0x0a, 0x07, 0x0a, 0x0b, 0x96, 0x04, 0xa1,
    0x3d, 0x0a, 0x0c, 0x0a, 0x0e, 0x96, 0x06, 0xa9, 0x73, 0x74, 0x64, 0x3a, 0x3a, 0x6f, 0x70, 0x65,
    0x6e, 0x0a, 0x0e, 0x0a, 0x17, 0x96, 0x05, 0xa1, 0x28, 0x0a, 0x17, 0x0a, 0x18, 0x96, 0x06, 0xa4,
    0x61, 0x72, 0x67, 0x73, 0x0a, 0x18, 0x0a, 0x1c, 0x96, 0x05, 0xa1, 0x5b, 0x0a, 0x1c, 0x0a, 0x1d,
    0x96, 0x07, 0xa1, 0x30, 0x0a, 0x1d, 0x0a, 0x1e, 0x96, 0x05, 0xa1, 0x5d, 0x0a, 0x1e, 0x0a, 0x1f,
    0x96, 0x05, 0xa1, 0x2c, 0x0a, 0x1f, 0x0a, 0x20, 0x96, 0x09, 0xa1, 0x72, 0x0a, 0x21, 0x0a, 0x24,
    0x96, 0x05, 0xa1, 0x29, 0x0a, 0x24, 0x0a, 0x25, 0x96, 0x05, 0xa1, 0x3b, 0x0a, 0x25, 0x0a, 0x26,
    0x96, 0x03, 0xa3, 0x6c, 0x65, 0x74, 0x0b, 0x03, 0x0b, 0x06, 0x96, 0x06, 0xa7, 0x63, 0x6f, 0x6e,
    0x74, 0x65, 0x6e, 0x74, 0x0b, 0x07, 0x0b, 0x0e, 0x96, 0x04, 0xa1, 0x3d, 0x0b, 0x0f, 0x0b, 0x11,
    0x96, 0x06, 0xa4, 0x66, 0x69, 0x6c, 0x65, 0x0b, 0x11, 0x0b, 0x15, 0x96, 0x04, 0xa1, 0x2e, 0x0b,
    0x15, 0x0b, 0x17, 0x96, 0x06, 0xa8, 0x72, 0x65, 0x61, 0x64, 0x5f, 0x61, 0x6c, 0x6c, 0x0b, 0x17,
    0x0b, 0x1e, 0x96, 0x05, 0xa1, 0x28, 0x0b, 0x1e, 0x0b, 0x1f, 0x96, 0x05, 0xa1, 0x29, 0x0b, 0x1f,
    0x0b, 0x20, 0x96, 0x05, 0xa1, 0x3b, 0x0b, 0x20, 0x0b, 0x21, 0x96, 0x06, 0xa4, 0x66, 0x69, 0x6c,
    0x65, 0x0c, 0x03, 0x0c, 0x07, 0x96, 0x04, 0xa1, 0x2e, 0x0c, 0x07, 0x0c, 0x09, 0x96, 0x06, 0xa5,
    0x63, 0x6c, 0x6f, 0x73, 0x65, 0x0c, 0x09, 0x0c, 0x0d, 0x96, 0x05, 0xa1, 0x28, 0x0c, 0x0d, 0x0c,
    0x0e, 0x96, 0x05, 0xa1, 0x29, 0x0c, 0x0e, 0x0c, 0x0f, 0x96, 0x05, 0xa1, 0x3b, 0x0c, 0x0f, 0x0c,
    0x10, 0x96, 0x03, 0xa3, 0x6c, 0x65, 0x74, 0x0e, 0x03, 0x0e, 0x06, 0x96, 0x06, 0xa1, 0x78, 0x0e,
    0x07, 0x0e, 0x08, 0x96, 0x04, 0xa1, 0x3d, 0x0e, 0x09, 0x0e, 0x0b, 0x96, 0x08, 0xd9, 0x41, 0x31,
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x31, 0x33, 0x39, 0x33, 0x37, 0x32, 0x31, 0x31, 0x36, 0x39, 0x35, 0x39, 0x34, 0x31, 0x34, 0x30,
    0x39, 0x39, 0x31, 0x33, 0x30, 0x37, 0x31, 0x32, 0x30, 0x36, 0x34, 0x2e, 0x30, 0x30, 0x30, 0x30,
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x01, 0x01, 0x01, 0x03, 0x96, 0x04, 0xa1, 0x2d, 0x0e, 0x1f, 0x0e, 0x21, 0x96, 0x07, 0xa3, 0x35,
    0x39, 0x34, 0x0e, 0x21, 0x0e, 0x2d, 0x96, 0x05, 0xa1, 0x3b, 0x0e, 0x2d, 0x0e, 0x2e, 0x96, 0x06,
    0xa5, 0x70, 0x72, 0x69, 0x6e, 0x74, 0x0f, 0x03, 0x0f, 0x08, 0x96, 0x05, 0xa1, 0x28, 0x0f, 0x08,
    0x0f, 0x09, 0x96, 0x06, 0xa1, 0x78, 0x0f, 0x09, 0x0f, 0x0a, 0x96, 0x05, 0xa1, 0x2c, 0x0f, 0x0a,
    0x0f, 0x0b, 0x96, 0x06, 0xa7, 0x63, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x0f, 0x0c, 0x0f, 0x13,
    0x96, 0x04, 0xa1, 0x2e, 0x0f, 0x13, 0x0f, 0x15, 0x96, 0x06, 0xa8, 0x61, 0x73, 0x5f, 0x62, 0x79,
    0x74, 0x65, 0x73, 0x0f, 0x15, 0x0f, 0x1c, 0x96, 0x05, 0xa1, 0x28, 0x0f, 0x1c, 0x0f, 0x1d, 0x96,
    0x05, 0xa1, 0x29, 0x0f, 0x1d, 0x0f, 0x1e, 0x96, 0x04, 0xa1, 0x2e, 0x0f, 0x1e, 0x0f, 0x20, 0x96,
    0x06, 0xa9, 0x74, 0x72, 0x61, 0x6e, 0x73, 0x6d, 0x75, 0x74, 0x65, 0x0f, 0x20, 0x0f, 0x28, 0x96,
    0x05, 0xa1, 0x28, 0x0f, 0x28, 0x0f, 0x29, 0x96, 0x09, 0xa5, 0x65, 0x6e, 0x48, 0x45, 0x58, 0x0f,
    0x29, 0x0f, 0x30, 0x96, 0x05, 0xa1, 0x29, 0x0f, 0x30, 0x0f, 0x31, 0x96, 0x04, 0xa1, 0x2e, 0x0f,
    0x31, 0x0f, 0x33, 0x96, 0x06, 0xa4, 0x6a, 0x6f, 0x69, 0x6e, 0x0f, 0x33, 0x0f, 0x36, 0x96, 0x05,
    0xa1, 0x28, 0x0f, 0x36, 0x0f, 0x37, 0x96, 0x09, 0xa1, 0x3a, 0x0f, 0x37, 0x0f, 0x3a, 0x96, 0x05,
    0xa1, 0x29, 0x0f, 0x3a, 0x0f, 0x3b, 0x96, 0x04, 0xa1, 0x2e, 0x0f, 0x3b, 0x0f, 0x3d, 0x96, 0x06,
    0xa6, 0x61, 0x73, 0x5f, 0x73, 0x74, 0x72, 0x0f, 0x3d, 0x0f, 0x42, 0x96, 0x05, 0xa1, 0x28, 0x0f,
    0x42, 0x0f, 0x43, 0x96, 0x05, 0xa1, 0x29, 0x0f, 0x43, 0x0f, 0x44, 0x96, 0x05, 0xa1, 0x29, 0x0f,
    0x44, 0x0f, 0x45, 0x96, 0x05, 0xa1, 0x3b, 0x0f, 0x45, 0x0f, 0x46, 0x96, 0x03, 0xa3, 0x72, 0x65,
    0x74, 0x11, 0x03, 0x11, 0x06, 0x96, 0x07, 0xa1, 0x30, 0x11, 0x07, 0x11, 0x08, 0x96, 0x05, 0xa1,
    0x3b, 0x11, 0x08, 0x11, 0x09, 0x96, 0x05, 0xa1, 0x7d, 0x12, 0x01, 0x12, 0x02, 0x96, 0x01, 0xa0,
    0x00, 0x00, 0x00, 0x00};

TEST(MetaRoute, flag_use_json) {
  nit_stream_t* source =
      nit_from(fmemopen((void*)source_code.data(), source_code.size(), "rb"), true);
  ASSERT_NE(source, nullptr);

  char* output_buf = nullptr;
  size_t output_size = 0;
  FILE* output = open_memstream(&output_buf, &output_size);
  if (output == nullptr) {
    nit_fclose(source);
    FAIL() << "Failed to open memory stream.";
  }

  const char* options[] = {
      "meta",       /* Meta route */
      "-fuse-json", /* Output as JSON */
      NULL,         /* End of options */
  };

  if (!nit_cc(source, output, nullptr, 0, options)) {
    nit_fclose(source);
    fclose(output);
    free(output_buf);
    nit_deinit();

    FAIL() << "Failed to run nit_cc.";
  }

  nit_fclose(source);
  fclose(output);

  std::string_view output_code(output_buf, output_size);

  EXPECT_EQ(output_code, expected_json);

  free(output_buf);
  nit_deinit();
}

TEST(MetaRoute, flag_use_msgpack) {
  nit_stream_t* source =
      nit_from(fmemopen((void*)source_code.data(), source_code.size(), "rb"), true);
  ASSERT_NE(source, nullptr);

  char* output_buf = nullptr;
  size_t output_size = 0;
  FILE* output = open_memstream(&output_buf, &output_size);
  if (output == nullptr) {
    nit_fclose(source);
    FAIL() << "Failed to open memory stream.";
  }

  const char* options[] = {
      "meta",          /* Meta route */
      "-fuse-msgpack", /* Output as MessagePack */
      NULL,            /* End of options */
  };

  if (!nit_cc(source, output, nullptr, 0, options)) {
    nit_fclose(source);
    fclose(output);
    free(output_buf);
    nit_deinit();

    FAIL() << "Failed to run nit_cc.";
  }

  nit_fclose(source);
  fclose(output);

  std::string_view output_code(output_buf, output_size);

  EXPECT_EQ(output_code, std::string_view((const char*)expected_msgpack, sizeof(expected_msgpack)));

  free(output_buf);
  nit_deinit();
}