#include "fake-jni/jvm.h"

namespace FakeJni {
 jint InvokeInterface::getEnv(Jvm * vm, void ** penv, jint version) const {
  *penv = (void *)((JNIEnv *)&vm->getJniEnv());
  return JNI_OK;
 }

 jint InvokeInterface::destroyJavaVm(Jvm * vm) const {
  return vm->destroy();
 }
}