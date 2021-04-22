//
// Created by willow li on 2019/3/26.
//

#include <jni.h>
#include <cstring>
#include <cstdlib>
#include <string>
#include <sstream>
#include "config.h"
#include "jdata_complier.h"

using namespace std;

jstring jstring_complier::string_to_jstring(JNIEnv *env, const string &str) {
    return char_to_jstring(env, str.c_str());
}

string jstring_complier::jstring_to_string(JNIEnv *env, jstring jstr) {
    return string(jstring_to_char(env, jstr));
}

jstring jstring_complier::char_to_jstring(JNIEnv *env, const char *pat) {
    jclass strClass = (env)->FindClass("java/lang/String");
    jmethodID ctorID = (env)->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
    auto strLength = jsize(strlen(pat));
    jbyteArray bytes = (env)->NewByteArray(strLength);
    (env)->SetByteArrayRegion(bytes, 0, strLength, (jbyte *) pat);
    jstring encoding = (env)->NewStringUTF("GB2312");
    return (jstring) (env)->NewObject(strClass, ctorID, bytes, encoding);
}

char *jstring_complier::jstring_to_char(JNIEnv *env, jstring jstr) {
    char *rtn = nullptr;
    jclass clsstring = env->FindClass("java/lang/String");
    jstring strencode = env->NewStringUTF("GB2312");
    jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
    auto barr = (jbyteArray) env->CallObjectMethod(jstr, mid, strencode);
    auto alen = (size_t) (env->GetArrayLength(barr));
    jbyte *ba = env->GetByteArrayElements(barr, JNI_FALSE);
    if (alen > 0) {
        rtn = (char *) malloc(alen + 1);
        memcpy(rtn, ba, alen);
        rtn[alen] = 0;
    }
    env->ReleaseByteArrayElements(barr, ba, 0);
    return rtn;
}

string jstring_complier::int2str(const int &int_temp) {
    stringstream stream;
    stream << int_temp;
    return stream.str();   //此处也可以用 stream>>string_temp
}

int jstring_complier::jbyteArray_to_int(JNIEnv *env, jbyteArray bytes_data) {
    jbyte *bytes = (env)->GetByteArrayElements(bytes_data, nullptr);
    int int_value = bytes[0] & 0xFF;
    int_value |= ((bytes[1] << 8) & 0xFF00);
    int_value |= ((bytes[2] << 16) & 0xFF0000);
    int_value |= ((bytes[3] << 24) & 0xFF000000);
    (env)->ReleaseByteArrayElements(bytes_data, bytes, 0);
    return int_value;
}

jintArray jstring_complier::jbyteArray_to_jintArray(JNIEnv *env, jbyteArray bytes_data) {
    jsize data_length = (env)->GetArrayLength(bytes_data);
    jsize true_length = data_length / 4;
    jbyte *bytes = (env)->GetByteArrayElements(bytes_data, nullptr);
    jintArray jintArray_result = (env)->NewIntArray(true_length);
    jint buf[true_length];
    for (int i = 0; i < true_length; ++i) {
        int start = i * 4;
        int int_value = bytes[start + 0] & 0xFF;
        int_value |= ((bytes[start + 1] << 8) & 0xFF00);
        int_value |= ((bytes[start + 2] << 16) & 0xFF0000);
        int_value |= ((bytes[start + 3] << 24) & 0xFF000000);
        buf[i] = int_value;
    }
    (env)->SetIntArrayRegion(jintArray_result, 0, true_length, buf);
    (env)->ReleaseByteArrayElements(bytes_data, bytes, 0);
    return jintArray_result;
}

array2d<rgb_pixel> jstring_complier::jbyteArray_to_array2d(JNIEnv *env,
                                                           jbyteArray bytes_data,
                                                           jint image_height,
                                                           jint image_widht) {
    jsize data_length = (env)->GetArrayLength(bytes_data);
    jbyte *bytes = (env)->GetByteArrayElements(bytes_data, nullptr);
    array2d<rgb_pixel> frame;
    frame.set_size(image_height, image_widht);
    for (int i = 0; i < image_height; i++) {
        for (int j = 0; j < image_widht; j++) {
            int index = i * image_widht + j;
            int start = index * 4;
            if (start >= data_length) {
                break;
            }
            /*frame[i][j] = dlib::rgb_pixel(
                    (unsigned char) (bytes[start + 2]),
                    (unsigned char) (bytes[start + 1]),
                    (unsigned char) (bytes[start + 0])
            );*/
            frame[i][j] = dlib::rgb_pixel(
                    (unsigned char) (bytes[start + 0]),
                    (unsigned char) (bytes[start + 1]),
                    (unsigned char) (bytes[start + 2])
            );
            /*if (OPEN_LOG && (index == 0 || index == 1)) {
                std::string plog =
                        "(" + jstring_complier::int2str(i) + ","
                        + jstring_complier::int2str(j) + ") "
                        + jstring_complier::int2str((int) (frame[i][j].red)) + ","
                        + jstring_complier::int2str((int) (frame[i][j].green)) + ","
                        + jstring_complier::int2str((int) (frame[i][j].blue)) + "  alpha = "
                        + jstring_complier::int2str((int) (bytes[start + 3]));
                char buf[plog.size()];
                strcpy(buf, plog.c_str());
                LOGI("%s", buf);
                plog.clear();
            }*/
        }
    }
    return frame;
}

array2d<unsigned char> jstring_complier::jbyteArray_to_array2dGrayscale(JNIEnv *env,
                                                                        jbyteArray bytes_data,
                                                                        jint image_height,
                                                                        jint image_widht) {
    jsize data_length = (env)->GetArrayLength(bytes_data);
    jbyte *bytes = (env)->GetByteArrayElements(bytes_data, nullptr);
    array2d<unsigned char> frame;
    frame.set_size(image_height, image_widht);
    int final_i = 0;
    int final_j = 0;
    for (int i = 0; i < image_height; i++) {
        for (int j = 0; j < image_widht; j++) {
            int index = i * image_widht + j;
            int start = index * 4;
            if (start >= data_length) {
                break;
            }
            int alpha = bytes[start + 3] & 0xFF;
            int r = ((bytes[start + 0]) & 0xFF);
            int g = ((bytes[start + 1]) & 0xFF);
            int b = ((bytes[start + 2]) & 0xFF);
            frame[i][j] = (unsigned char) (
                    (r * 19595 + g * 38469 + b * 7472) >> 16
            );
            if (OPEN_LOG && (index == 0 || index == 1)) {
                std::string plog =
                        "(" + jstring_complier::int2str(i) + ","
                        + jstring_complier::int2str(j) + ") "
                        + jstring_complier::int2str((int) (frame[i][j]));
                char buf[plog.size()];
                strcpy(buf, plog.c_str());
                LOGI("%s", buf);
                plog.clear();
            }
            final_j = j;
        }
        final_i = i;
    }
    if (OPEN_LOG) {
        int index = final_i * image_widht + final_j;
        int start = index * 4;
        std::string plog =
                "data length :: " + jstring_complier::int2str(data_length) +
                "r finish at :: " + jstring_complier::int2str(start) +
                "e finish at :: " + jstring_complier::int2str(start + 4);
        char buf[plog.size()];
        strcpy(buf, plog.c_str());
        LOGI("%s", buf);
        plog.clear();
    }
    return frame;
}

array2d<unsigned char> jstring_complier::jbyteArray_to_array2dOrigin(JNIEnv *env,
                                                                     jbyteArray bytes_data,
                                                                     jint image_height,
                                                                     jint image_widht) {
    jsize data_length = (env)->GetArrayLength(bytes_data);
    jbyte *bytes = (env)->GetByteArrayElements(bytes_data, nullptr);
    array2d<unsigned char> frame;
    frame.set_size(image_height, image_widht);
    for (int i = 0; i < image_height; ++i) {
        for (int j = 0; j < image_widht; j++) {
            int index = i * image_widht + j;
            int start = index * 4;
            if (start >= data_length) {
                break;
            }
            unsigned char value = 0;
            value |= ((bytes[start + 0]) & 0xFF);
            value |= ((bytes[start + 1] << 8) & 0xFF00);
            value |= ((bytes[start + 2] << 16) & 0xFF0000);
            value |= ((bytes[start + 3] << 24) & 0xFF000000);
            frame[i][j] = value;
            if (OPEN_LOG && (index == 0 || index == 1)) {
                std::string plog =
                        "(" + jstring_complier::int2str(i) + ","
                        + jstring_complier::int2str(j) + ") "
                        + jstring_complier::int2str((int) (frame[i][j]));
                char buf[plog.size()];
                strcpy(buf, plog.c_str());
                LOGI("%s", buf);
                plog.clear();
            }
        }
    }
    return frame;
}
/********************************************************************************
功能      : YUV数据转转换为matrix
输入      : YUV数据，图片高，宽，补位，格式
返回      : matrix<rgb_pixel>, 图片YUV数据转换成matrix 格式返回
*********************************************************************************/
/*

matrix<rgb_pixel> yuv_2_matrix(unsigned char *yuvData,
                               int iHeight,
                               int iWidth
) {
    matrix<rgb_pixel> img;
    img.set_size(iHeight, iWidth);

    if (nullptr == yuvData) {
        cout << "YUV data error, Parameter is NULL !" << endl;
        exit(1);
    }
    int yLen = iHeight * iWidth;
    //int uLen = iHeight*iWidth/2;
    //int vLen = uLen;

    auto yData = new unsigned char[yLen];
    //int *uData = new int[uLen];
    //int *vData = new int[vLen];

    for (int i = 0; i < iHeight * iWidth; i++) {
        yData[i] = yuvData[i];
    }

    int k = 0;
    for (int i = 0; i < iHeight; i++) {
        for (int j = 0; j < iWidth; j++, k++) {
            img(i, j) = rgb_pixel(
                    (unsigned char) yData[k],
                    (unsigned char) yData[k],
                    (unsigned char) yData[k]
            );
        }
    }
    delete yData;

    return img;
}*/
