#include <iostream>

#include "fake-jni/jvm.h"
#include "fake-jni/array.h"
#include "fake-jni/string.h"

using namespace FakeJni;

class ExampleClass : public JObject {
public:
 DEFINE_CLASS_NAME("com/example/ExampleClass")

 using cast = CX::ExplicitCastGenerator<ExampleClass, JClass, JObject>;

 JInt exampleField1;
 JString exampleField2;

 //This constructor is visible to the JVM
 ExampleClass() :
  exampleField1(10),
  exampleField2("Hello World!")
 {}

 //This constructor is visible to the JVM
 ExampleClass(JInt, JString *) :
  exampleField1(0),
  exampleField2{""}
 {}

 //This constructor is visible to the JVM
 ExampleClass(JDouble, ExampleClass *) :
  exampleField1(0),
  exampleField2{""}
 {}

 //This constructor is not visible to the JVM and thus no compile-time error is generated
 //for the illegal constructor prototype
 ExampleClass(JBooleanArray arr):
  exampleField1(-1),
  exampleField2("This constructor is not registered on the JVM!")
 {}

 JInt exampleFunction() {
  return exampleField1;
 }

 JString* getMyString() {
  return &exampleField2;
 }

 inline static void exampleStaticFunction(JDouble d) {
  std::cout << "From ExampleClass: " << d << std::endl;
 }

 static void imNotLinkedToAnything(const char * str) {
  std::cout << str << std::endl;
 }
};

static void outOfLineMemberFunction() {
 std::cout << "I am JVM static!" << std::endl;
}

BEGIN_NATIVE_DESCRIPTOR(ExampleClass)
 //Link member functions
 {Function<&ExampleClass::exampleFunction> {}, "exampleFunction"},
 {Function<&ExampleClass::getMyString> {}, "getMyString"},
 //Link static function
 {Function<&exampleStaticFunction> {}, "exampleStaticFunction", JMethodID::STATIC},
 //Link member function
 {Function<&exampleStaticFunction> {}, "theSameFunctionButNotStaticAndWithADifferentName"},
 //Link static function
 {Function<&outOfLineMemberFunction> {}, "woahTheNameIsDifferentAgain"},
 //Link member fields
 {Field<&ExampleClass::exampleField1> {}, "exampleField1"},
 {Field<&ExampleClass::exampleField2> {}, "exampleField2"},
 //Register constructors
 {Constructor<ExampleClass> {}},
 {Constructor<ExampleClass, JInt, JString*> {}},
 {Constructor<ExampleClass, JDouble, ExampleClass *> {}}
END_NATIVE_DESCRIPTOR

jint test(JNIEnv *env, jobject obj, JObject * a, JDouble b, JDouble c) {
 printf("Obj type: %s\n", a->getClass().getName());
 printf("0x%lx %f %f\n", (intptr_t)a, b, c);
 return 314;
}

DECLARE_NATIVE_ARRAY_DESCRIPTOR(JString)

JString * test2(JNIEnv *env, jobject obj, JString *s1, JArray<JString> *astr, JString *s2, JBooleanArray *b) {
 return JString::EMPTY;
}

JClass dummy{"bruh"};

DEFINE_NATIVE_ARRAY_DESCRIPTOR(JString)

//fake-jni in action
int main(int argc, char **argv) {
 JNINativeMethod nm {
  "test",
  "(L;DD)I",
  (void *)&test
 };

 JNINativeMethod nm2 {
  "test2",
  "([Ljava/lang/String;[Ljava/lang/String;Ljava/lang/String;[B)Ljava/lang/String;",
  (void *)&test2
 };

 //Make a JString
 JString test{"Hello World!"};

 //Create a shiny new fake JVM instance
 Jvm vm;

 auto mid = new JMethodID{&nm};
 dummy.registerMethod(mid);
 auto mid2 = new JMethodID{&nm2};
 dummy.registerMethod(mid2);

 auto result = mid->invoke<jint>(&vm, &dummy, &dummy, (JDouble)2, (JDouble)3);
 printf("FUNCTION RETURNED: %d(0x%x)\n", result, result);
 printf("JString::EMPTY -> 0x%lx\n", (intptr_t)JString::EMPTY);
 auto result2 = mid2->invoke<JString *>(&vm, &dummy, &dummy, nullptr, nullptr, nullptr, nullptr);
 printf("TEST2 RETURNED: 0x%lx\n", (intptr_t)result2);

 //Register ExampleClass on the JVM instance
 vm.registerClass<ExampleClass>();

 //Attach this binary as a native library
 //no path to current binary, no options, no custom library dl functions
 vm.attachLibrary("");

 //Start fake-jni
// vm->start();

 //Not necessary
 vm.unregisterClass<ExampleClass>();

 //Clean up
 vm.destroy();

 return 0;
}