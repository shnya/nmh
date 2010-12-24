/*
 * WRITTEN BY Masahiko Higashiyama in 2010.
 *
 * THIS CODE IS IN PUBLIC DOMAIN.
 * THIS SOFTWARE IS COMPLETELY FREE TO COPY, MODIFY AND/OR RE-DISTRIBUTE.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <CUnit/CUnit.h>
#include <CUnit/Console.h>
#include <CUnit/Basic.h>
#include <iconv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "nmh.h"

int convert(const char *to_code, const char *input, char **output);
void test_detect_utf16(void);
void test_detect_utf32(void);
void test_detect_jis(void);
void test_detect_utf8n(void);
void test_detect_utf8(void);
void test_detect_sjis(void);
void test_detect_euc(void);

const char * const test_str = "大きなノッポの古時計";

// From iconv official sample code
// http://www.gnu.org/s/libc/manual/html_node/iconv-Examples.html
int convert(const char *to_code, const char *input, char **output){
  char *inbuf, *iptr, *outbuf, *wptr;
  iconv_t cd;
  size_t nconv, inlen, avail;

  *output = NULL;

  cd =  iconv_open(to_code, "UTF-8");
  if(cd == (iconv_t)-1){
    perror("iconv error");
    return -1;
  }

  iptr = inbuf = strdup(input);
  inlen = strlen(inbuf);
  avail = inlen * 4 + 1;
  wptr = outbuf = (char *)malloc(avail * sizeof(char));

  nconv = iconv(cd, &iptr, &inlen, &wptr, &avail);
  if(nconv == (size_t) -1){
    perror("iconv error");
    free(inbuf);
    free(outbuf);
    return -1;
  }
  iconv(cd, NULL, NULL, &wptr, &avail);

  if(avail >= sizeof(char)){
    *wptr = '\0';
  }else{
    perror("iconv error");
    free(inbuf);
    free(outbuf);
    return -1;
  }

  if(iconv_close(cd) != 0){
    perror("iconv error");
    free(inbuf);
    free(outbuf);
    return -1;
  }

  *output = outbuf;

  return 1;
}

void test_detect_utf16(void){
  char *output;
  enum NMH_CHAR_CODE code;
  int result;
  float score;

  result = convert("UTF-16", test_str, &output);
  CU_ASSERT_EQUAL(result,1);

  score = nmh_is_utf16((unsigned char *)output, strlen(output));
  CU_ASSERT_DOUBLE_EQUAL(0.5, score, 0.1);

  code = nmh_code_detect(output, strlen(output));
  CU_ASSERT_EQUAL(code,NMH_CHAR_CODE_UTF16LE);
  free(output);

}

void test_detect_utf32(void){
  char *output;
  enum NMH_CHAR_CODE code;
  int result;
  float score;

  result = convert("UTF-32", test_str, &output);
  CU_ASSERT_EQUAL(result,1);

  score = nmh_is_utf32((unsigned char *)output, 8);
  CU_ASSERT_DOUBLE_EQUAL(0.5, score, 0.1);

  code = nmh_code_detect(output, 8);
  CU_ASSERT_EQUAL(code,NMH_CHAR_CODE_UTF32LE);
  free(output);
}

void test_detect_jis(void){
  char *output = NULL;
  enum NMH_CHAR_CODE code;
  int result = convert("ISO-2022-JP", test_str, &output);
  float score;

  CU_ASSERT_EQUAL(result,1);

  score = nmh_is_jis((unsigned char *)output, strlen(output));
  CU_ASSERT_DOUBLE_EQUAL(1.0, score, 0.1);

  code = nmh_code_detect(output, strlen(output));
  CU_ASSERT_EQUAL(code,NMH_CHAR_CODE_JIS);

  free(output);
}

void test_detect_utf8n(void){
  char *output, *output2;
  enum NMH_CHAR_CODE code;
  float score;
  int result = convert("UTF-8", test_str, &output2);
  CU_ASSERT_EQUAL(result,1);

  output = (char *)malloc(sizeof(char) * strlen(output2) + 4);
  strncpy(output + 3, test_str, strlen(output2) + 1);
  free(output2);
  output[0] = 0xEF;
  output[1] = 0xBB;
  output[2] = 0xBF;

  score = nmh_is_utf8n((unsigned char *)output, strlen(output));
  CU_ASSERT_DOUBLE_EQUAL(1.0, score, 0.1);
  score = nmh_is_utf8((unsigned char *)output, strlen(output));
  CU_ASSERT_DOUBLE_EQUAL(1.5, score, 0.1);

  code = nmh_code_detect(output, strlen(output));
  CU_ASSERT_EQUAL(code,NMH_CHAR_CODE_UTF8N);

  free(output);
}


void test_detect_utf8(void){
  char *output;
  enum NMH_CHAR_CODE code;
  int result = convert("UTF-8", test_str, &output);
  float score;

  CU_ASSERT_EQUAL(result,1);

  score = nmh_is_utf8((unsigned char *)output, strlen(output));
  CU_ASSERT_DOUBLE_EQUAL(0.5, score, 0.1);

  code = nmh_code_detect(output, strlen(output));
  CU_ASSERT_EQUAL(code,NMH_CHAR_CODE_UTF8);

  free(output);
}

void test_detect_sjis(void){
  char *output;
  enum NMH_CHAR_CODE code;
  int result = convert("SJIS", test_str, &output);
  float score;

  CU_ASSERT_EQUAL(result,1);

  score = nmh_is_sjis((unsigned char *)output, strlen(output));
  CU_ASSERT_DOUBLE_EQUAL(1.0, score, 0.1);

  code = nmh_code_detect(output, strlen(output));
  CU_ASSERT_EQUAL(code,NMH_CHAR_CODE_SJIS);

  free(output);
}

void test_detect_euc(void){
  char *output;
  enum NMH_CHAR_CODE code;
  int result = convert("EUC-JP", test_str, &output);
  float score;

  CU_ASSERT_EQUAL(result,1);

  score = nmh_is_euc((unsigned char *)output, strlen(output));
  CU_ASSERT_DOUBLE_EQUAL(1.0, score, 0.1);

  code = nmh_code_detect(output, strlen(output));
  CU_ASSERT_EQUAL(code,NMH_CHAR_CODE_EUC);

  free(output);
}

void test_detect_ascii(void){
  char *output = "abcde";
  enum NMH_CHAR_CODE code;
  float score;

  score = nmh_is_ascii((unsigned char *)output, strlen(output));
  CU_ASSERT_DOUBLE_EQUAL(1.0, score, 0.1);

  code = nmh_code_detect(output, strlen(output));
  CU_ASSERT_EQUAL(code,NMH_CHAR_CODE_ASCII);
}


int main(){
  CU_pSuite pSuite = NULL;

  /* initialize the CUnit test registry */
  if (CUE_SUCCESS != CU_initialize_registry())
    return CU_get_error();

  /* add a suite to the registry */
  pSuite = CU_add_suite("Suite", NULL, NULL);
  if (NULL == pSuite) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  if ((NULL == CU_add_test(pSuite, "Test_jis", test_detect_jis)) ||
      (NULL == CU_add_test(pSuite, "Test_utf16", test_detect_utf16)) ||
      (NULL == CU_add_test(pSuite, "Test_utf32", test_detect_utf32)) ||
      (NULL == CU_add_test(pSuite, "Test_utf8n", test_detect_utf8n)) ||
      (NULL == CU_add_test(pSuite, "Test_utf8", test_detect_utf8)) ||
      (NULL == CU_add_test(pSuite, "Test_sjis", test_detect_sjis)) ||
      (NULL == CU_add_test(pSuite, "Test_euc", test_detect_euc)) ||
      (NULL == CU_add_test(pSuite, "Test_ascii", test_detect_ascii))){
    CU_cleanup_registry();
    return CU_get_error();
  }

  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  CU_cleanup_registry();

  return CU_get_error();
}
