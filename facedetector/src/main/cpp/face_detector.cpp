//
// 检测器方法实现
// Created by willow li on 2019/3/26.
//

#include <dlib/image_processing/frontal_face_detector.h>
#include <include/opencv2/core/mat.hpp>
#include <include/opencv2/imgproc/imgproc_c.h>
#include <include/opencv2/imgproc/types_c.h>
#include <src/main/cpp/dlib/opencv/cv_image.h>
#include <include/opencv2/imgproc.hpp>
#include "config.h"
#include "face_detector.h"

using namespace out_cast_detector;
using namespace cv;
using namespace dlib;

face_detector::face_detector() = default;

void face_detector::init_face_detector(JNIEnv *env, jstring predictor_path) {
    LOGI("init_face_detector start");
    string path = jstring_complier::jstring_to_string(env, predictor_path);
    detector = dlib::get_frontal_face_detector();
    predictor = shape_predictor();
    deserialize(path) >> predictor;
    // deserialize(predictor, path);

    jclass pointFClazz = (env)->FindClass("android/graphics/PointF");
    jclass faceInfoClass = (env)->FindClass("com/video/facedetector/FaceInfo");
    mPointClass = (jclass) (env)->NewGlobalRef(pointFClazz);
    mFaceInfoClass = (jclass) (env)->NewGlobalRef(faceInfoClass);
    pointConstructID = (env)->GetMethodID(mPointClass, "<init>", "()V");
    faceConstructID = (env)->GetMethodID(mFaceInfoClass, "<init>", "()V");
    LOGI("init_face_detector finish");
}

jobjectArray face_detector::do_face_detect_action(JNIEnv *env,
                                                  jbyteArray image_data,
                                                  jint image_height,
                                                  jint image_widht) {
    try {
        if (image_data == nullptr || image_height <= 0 || image_widht <= 0) {
            return nullptr;
        }
        // 灰度取值
        LOGI("数据转换 start");
        array2d<unsigned char> frame =
                jstring_complier::jbyteArray_to_array2dGrayscale(
                        env,
                        image_data,
                        image_height,
                        image_widht
                );
        LOGI("数据转换 finish");

        // 人脸检测
        LOGI("人脸检测 start");
        detector = dlib::get_frontal_face_detector();
        std::vector<dlib::rectangle> rects = detector(frame);
        LOGI("至少有人脸? %i", (int) jsize(rects.size()));
        jobjectArray final_result =
                (env)->NewObjectArray(
                        jsize(rects.size()),
                        mFaceInfoClass,
                        nullptr
                );
        for (int i = 0; i < rects.size(); ++i) {
            full_object_detection faces_landmark = predictor(frame, rects[i]);
            jobjectArray keymarks = (env)->NewObjectArray(68, mPointClass, nullptr);
            for (int index = 0; index < 68; index++) {
                point p = faces_landmark.part((unsigned long) (index));
                jobject tempPoint = (env)->NewObject(mPointClass, pointConstructID);
                jfieldID px = (env)->GetFieldID(mPointClass, "x", "F");
                jfieldID py = (env)->GetFieldID(mPointClass, "y", "F");
                (env)->SetFloatField(tempPoint, px, (float) p.x());
                (env)->SetFloatField(tempPoint, py, (float) p.y());
                (env)->SetObjectArrayElement(keymarks, index, tempPoint);
                (env)->DeleteLocalRef(tempPoint);
            }
            jobject tempFaceInfo = (env)->NewObject(mFaceInfoClass, faceConstructID);
            jfieldID marksId = (env)->GetFieldID(mFaceInfoClass, "mKeyPoints",
                                                 "[Landroid/graphics/PointF;");
            (env)->SetObjectField(tempFaceInfo, marksId, keymarks);
            (env)->SetObjectArrayElement(final_result, i, tempFaceInfo);
            LOGI("单个人脸数据释放前");
            (env)->DeleteLocalRef(tempFaceInfo);
            (env)->DeleteLocalRef(keymarks);
        }
        LOGI("人脸检测 finish");
        rects.clear();
        frame.clear();
        return final_result;

    } catch (exception &e) {
        LOGE("%s", e.what());
    }
    return nullptr;
}


jobjectArray face_detector::do_face_detect_action_mat(JNIEnv *env,
                                                      const cv::Mat &image_data,
                                                      jint image_height,
                                                      jint image_widht) {
    /*try {
        if (image_data.empty() || image_height <= 0 || image_widht <= 0) {
            return nullptr;
        }
        // 灰度取值
        LOGI("数据转换 start");
        if (image_data.channels() == 1) {
            cv::cvtColor(image_data, image_data, CV_GRAY2BGR);
        }

        dlib::cv_image<dlib::bgr_pixel> frame(image_data);
        LOGI("数据转换 finish");

        // 人脸检测
        LOGI("人脸检测 start");
        std::vector<dlib::rectangle> rects = detector(frame);
        LOGI("至少有人脸? %i", (int) jsize(rects.size()));
        jobjectArray final_result =
                (env)->NewObjectArray(
                        jsize(rects.size()),
                        mFaceInfoClass,
                        nullptr
                );
        for (int i = 0; i < rects.size(); ++i) {
            full_object_detection faces_landmark = predictor(frame, rects[i]);
            jobjectArray keymarks = (env)->NewObjectArray(68, mPointClass, nullptr);
            for (int index = 0; index < 68; index++) {
                point p = faces_landmark.part((unsigned long) (index));
                jobject tempPoint = (env)->NewObject(mPointClass, pointConstructID);
                jfieldID px = (env)->GetFieldID(mPointClass, "x", "F");
                jfieldID py = (env)->GetFieldID(mPointClass, "y", "F");
                (env)->SetFloatField(tempPoint, px, (float) p.x());
                (env)->SetFloatField(tempPoint, py, (float) p.y());
                (env)->SetObjectArrayElement(keymarks, index, tempPoint);
                (env)->DeleteLocalRef(tempPoint);
            }
            jobject tempFaceInfo = (env)->NewObject(mFaceInfoClass, faceConstructID);
            jfieldID marksId = (env)->GetFieldID(mFaceInfoClass, "mKeyPoints",
                                                 "[Landroid/graphics/PointF;");
            (env)->SetObjectField(tempFaceInfo, marksId, keymarks);
            (env)->SetObjectArrayElement(final_result, i, tempFaceInfo);
            LOGI("单个人脸数据释放前");
            (env)->DeleteLocalRef(tempFaceInfo);
            (env)->DeleteLocalRef(keymarks);
        }
        LOGI("人脸检测 finish");
        rects.clear();
        return final_result;

    } catch (exception &e) {
        LOGE("%s", e.what());
    }*/
    return nullptr;
}

/*暴露方法=======================================================================================*/
face_detector *current_detector = nullptr;

static void out_cast_detector::do_init(JNIEnv *env,
                                       jobject job,
                                       jstring predictor_path) {
    LOGI("init_face_detector proxy start");
    if (current_detector != nullptr) {
        current_detector->init_face_detector(env, predictor_path);
    }
    LOGI("init_face_detector proxy finish");
}


static jobjectArray out_cast_detector::do_detect(JNIEnv *env,
                                                 jobject obj,
                                                 jbyteArray image_data,
                                                 jint image_height,
                                                 jint image_widht) {
    LOGI("do_detect proxy");
    if (current_detector != nullptr) {
        return current_detector->do_face_detect_action(env, image_data, image_height, image_widht);
    }
    return nullptr;
}

static jobjectArray out_cast_detector::do_detect_mat(JNIEnv *env,
                                                     const cv::Mat &image_data,
                                                     jint image_height,
                                                     jint image_widht) {
    LOGI("do_detect proxy");
    if (current_detector != nullptr) {
        return current_detector->do_face_detect_action_mat(env, image_data, image_height,
                                                           image_widht);
    }
    return nullptr;
}

/*动态注册=======================================================================================*/

// 方法声明
static const char *jniClassName = "com/video/facedetector/FaceDetectorManager";
static const JNINativeMethod provide_methods[] = {
        {"initFaceDetector",      "(Ljava/lang/String;)V",                      (void *) out_cast_detector::do_init},
        {"doFaceDetectAction",    "([BII)[Lcom/video/facedetector/FaceInfo;", (jobjectArray *) out_cast_detector::do_detect},
        {"doFaceDetectMatAction", "([BII)[Lcom/video/facedetector/FaceInfo;", (jobjectArray *) out_cast_detector::do_detect_mat},
};

// 此函数通过调用RegisterNatives方法来注册我们的函数
static int registerNativeMethods(JNIEnv *env,
                                 const char *className,
                                 const JNINativeMethod *getMethods,
                                 int methodsNum) {
    jclass clazz;
    clazz = (env)->FindClass(className);
    if (clazz == nullptr) {
        return JNI_FALSE;
    }
    if ((env)->RegisterNatives(clazz, getMethods, methodsNum) < 0) {
        return JNI_FALSE;
    }
    if (current_detector == nullptr) {
        current_detector = new face_detector();
    }
    return JNI_TRUE;
}

// 通用动态注册函数
static int register_android_face_detector(JNIEnv *env) {
    return registerNativeMethods(env,
                                 jniClassName,
                                 provide_methods,
                                 sizeof(provide_methods) / sizeof(provide_methods[0]));
}

/*动态注册：生命周期===================================================================================*/
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = nullptr;
    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }
    assert(env != nullptr);
    if (!register_android_face_detector(env)) {
        return -1;
    }
    return JNI_VERSION_1_6;
}


JNIEXPORT void JNI_OnUnload(JavaVM *vm, void *reserved) {

}