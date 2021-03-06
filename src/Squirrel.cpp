///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
// Copyright (c) 2009 Brandon Jones
// Copyright (c) 2011 Camilla Berglund <elmindreda@elmindreda.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any
// damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any
// purpose, including commercial applications, and to alter it and
// redistribute it freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you
//     must not claim that you wrote the original software. If you use
//     this software in a product, an acknowledgment in the product
//     documentation would be appreciated but is not required.
//
//  2. Altered source versions must be plainly marked as such, and
//     must not be misrepresented as being the original software.
//
//  3. This notice may not be removed or altered from any source
//     distribution.
//
///////////////////////////////////////////////////////////////////////

#include <wendy/Config.hpp>
#include <wendy/Core.hpp>
#include <wendy/Path.hpp>
#include <wendy/Resource.hpp>

#include <wendy/Squirrel.hpp>

#include <sqstdmath.h>
#include <sqstdstring.h>

#include <sstream>
#include <fstream>

#include <cstring>

namespace wendy
{

namespace
{

std::string escapeString(const SQChar* string)
{
  std::string result;
  result.reserve(std::strlen(string));

  for (const SQChar* c = string;  *c;  c++)
  {
    if (*c == '\t')
      result += "\\t";
    else if (*c == '\a')
      result += "\\a";
    else if (*c == '\b')
      result += "\\b";
    else if (*c == '\n')
      result += "\\n";
    else if (*c == '\r')
      result += "\\r";
    else if (*c == '\v')
      result += "\\v";
    else if (*c == '\f')
      result += "\\f";
    else
      result += *c;
  }

  return result;
}

void logErrorCallStack(HSQUIRRELVM vm)
{
  std::ostringstream stream;

  const SQChar* error = "Unknown error";

  sq_getlasterror(vm);
  if (sq_gettype(vm, -1) == OT_STRING)
    sq_getstring(vm, -1, &error);
  sq_pop(vm, 2);

  stream << error;

  SQInteger level = 1;
  SQStackInfos si;

  while (SQ_SUCCEEDED(sq_stackinfos(vm, level, &si)))
  {
    if (!si.funcname)
      si.funcname = "UNNAMED";
    if (!si.source)
      si.source = "UNNAMED";

    stream << '\n' << si.source << ':' << si.line << ':' << si.funcname;

    SQInteger index = 0;
    const SQChar* name;

    while ((name = sq_getlocal(vm, level, index)))
    {
      stream << "\n  " << name << " = ";

      switch (sq_gettype(vm, -1))
      {
        case OT_INTEGER:
        {
          SQInteger value;
          sq_getinteger(vm, -1, &value);
          stream << value;
          break;
        }

        case OT_FLOAT:
        {
          SQFloat value;
          sq_getfloat(vm, -1, &value);
          stream << value;
          break;
        }

        case OT_BOOL:
        {
          SQBool value;
          sq_getbool(vm, -1, &value);
          stream << (value ? "true" : "false");
          break;
        }

        case OT_STRING:
        {
          const SQChar* value;
          sq_getstring(vm, -1, &value);
          stream << '\"' << escapeString(value) << '\"';
          break;
        }

        case OT_USERDATA:
        case OT_USERPOINTER:
        {
          SQUserPointer value;
          sq_getuserpointer(vm, -1, &value);
          stream << value;
          break;
        }

        case OT_NULL:
          stream << "null";
          break;
        case OT_TABLE:
          stream << "{ ... }";
          break;
        case OT_ARRAY:
          stream << "[ ... ]";
          break;
        case OT_CLOSURE:
          stream << "function() [Squirrel]";
          break;
        case OT_NATIVECLOSURE:
          stream << "function() [C++]";
          break;
        case OT_GENERATOR:
          stream << "generator()";
          break;
        case OT_THREAD:
          stream << "thread";
          break;
        case OT_FUNCPROTO:
          stream << "whathuh";
          break;
        case OT_CLASS:
          stream << "class";
          break;
        case OT_INSTANCE:
          stream << "instance";
          break;
        case OT_WEAKREF:
          stream << "weakref";
          break;
        case OT_OUTER:
          stream << "huhwhat";
          break;
      }

      sq_poptop(vm);
      index++;
    }

    level++;
  }

  logError("%s", stream.str().c_str());
}

template <typename T>
void pushData(HSQUIRRELVM vm, const char* className, T value)
{
  SqTable root(SqTable::rootTable(vm));
  SqClass class_(root.get<SqClass>(className));
  SqNativeInstance<T> instance(class_.createInstance());
  *instance.native() = value;
  sq_pushobject(vm, instance.handle());
}

template <typename T>
T vecAdd(T a, T b) { return a + b; }
template <typename T>
T vecSub(T a, T b) { return a - b; }
template <typename T>
T vecMul(T a, T b) { return a * b; }
template <typename T>
T vecDiv(T a, T b) { return a / b; }
template <typename T>
T vecUnm(T v) { return -v; }
template <typename T>
std::string vecToString(T v) { return stringCast(v); }
template <typename T>
float vecDot(T a, T b) { return dot(a, b); }

template <typename T>
void registerVectorClass(SqVM& vm, const char* name)
{
  SqTable rootTable(vm.rootTable());

  SqDataClass<T> vecClass(vm);
  vecClass.addMethod("_add", &vecAdd<T>);
  vecClass.addMethod("_sub", &vecSub<T>);
  vecClass.addMethod("_mul", &vecMul<T>);
  vecClass.addMethod("_div", &vecDiv<T>);
  vecClass.addMethod("_unm", &vecUnm<T>);
  vecClass.addMethod("_tostring", &vecToString<T>);
  vecClass.addMethod("dot", &vecDot<T>);
  rootTable.addSlot(name, (SqClass) vecClass);
}

void registerCoreClasses(SqVM& vm)
{
  SqTable rootTable(vm.rootTable());

  registerVectorClass<vec2>(vm, "Vec2");
  registerVectorClass<vec3>(vm, "Vec3");
  registerVectorClass<vec4>(vm, "Vec4");
}

} /*namespace*/

namespace detail
{

template <>
bool get(HSQUIRRELVM vm, SQInteger index)
{
  SQBool value;
  sq_getbool(vm, index, &value);
  return value ? true : false;
}

template <>
void push(HSQUIRRELVM vm, bool value)
{
  sq_pushbool(vm, SQBool(value));
}

template <>
int get(HSQUIRRELVM vm, SQInteger index)
{
  SQInteger value;
  sq_getinteger(vm, index, &value);
  return int(value);
}

template <>
void push(HSQUIRRELVM vm, int value)
{
  sq_pushinteger(vm, SQInteger(value));
}

template <>
float get(HSQUIRRELVM vm, SQInteger index)
{
  SQFloat value;
  sq_getfloat(vm, index, &value);
  return float(value);
}

template <>
void push(HSQUIRRELVM vm, float value)
{
  sq_pushfloat(vm, SQFloat(value));
}

template <>
const char* get(HSQUIRRELVM vm, SQInteger index)
{
  const SQChar* value;
  sq_getstring(vm, index, &value);
  return value;
}

template <>
void push(HSQUIRRELVM vm, const char* value)
{
  sq_pushstring(vm, value, -1);
}

template <>
std::string get(HSQUIRRELVM vm, SQInteger index)
{
  const SQChar* value;
  sq_getstring(vm, index, &value);
  return std::string(value);
}

template <>
void push(HSQUIRRELVM vm, std::string value)
{
  sq_pushstring(vm, value.c_str(), -1);
}

template <>
SqObject get(HSQUIRRELVM vm, SQInteger index)
{
  return SqObject(vm, index);
}

template <>
void push(HSQUIRRELVM vm, SqObject value)
{
  sq_pushobject(vm, value.handle());
}

template <>
SqArray get(HSQUIRRELVM vm, SQInteger index)
{
  return SqArray(vm, index);
}

template <>
void push(HSQUIRRELVM vm, SqArray value)
{
  sq_pushobject(vm, value.handle());
}

template <>
SqTable get(HSQUIRRELVM vm, SQInteger index)
{
  return SqTable(vm, index);
}

template <>
void push(HSQUIRRELVM vm, SqTable value)
{
  sq_pushobject(vm, value.handle());
}

template <>
SqClass get(HSQUIRRELVM vm, SQInteger index)
{
  return SqClass(vm, index);
}

template <>
void push(HSQUIRRELVM vm, SqClass value)
{
  sq_pushobject(vm, value.handle());
}

template <>
SqInstance get(HSQUIRRELVM vm, SQInteger index)
{
  return SqInstance(vm, index);
}

template <>
void push(HSQUIRRELVM vm, SqInstance value)
{
  sq_pushobject(vm, value.handle());
}

template <>
vec2 get(HSQUIRRELVM vm, SQInteger index)
{
  vec2* value;
  sq_getinstanceup(vm, index, (SQUserPointer*) &value, nullptr);
  return *value;
}

template <>
void push(HSQUIRRELVM vm, vec2 value)
{
  pushData(vm, "Vec2", value);
}

template <>
vec3 get(HSQUIRRELVM vm, SQInteger index)
{
  vec3* value;
  sq_getinstanceup(vm, index, (SQUserPointer*) &value, nullptr);
  return *value;
}

template <>
void push(HSQUIRRELVM vm, vec3 value)
{
  pushData(vm, "Vec3", value);
}

template <>
vec4 get(HSQUIRRELVM vm, SQInteger index)
{
  vec4* value;
  sq_getinstanceup(vm, index, (SQUserPointer*) &value, nullptr);
  return *value;
}

template <>
void push(HSQUIRRELVM vm, vec4 value)
{
  pushData(vm, "Vec4", value);
}

} /*namespace detail*/

SqVM::SqVM(ResourceCache& cache):
  m_cache(cache),
  m_vm(nullptr)
{
  m_vm = sq_open(1024);

  sq_setforeignptr(m_vm, nullptr);
  sq_setprintfunc(m_vm, onLogMessage, onLogError);
  sq_setcompilererrorhandler(m_vm, onCompilerError);

  sq_pushroottable(m_vm);
  sqstd_register_mathlib(m_vm);
  sqstd_register_stringlib(m_vm);
  sq_newclosure(m_vm, onRuntimeError, 0);
  sq_seterrorhandler(m_vm);
  sq_poptop(m_vm);

  registerCoreClasses(*this);
}

SqVM::~SqVM()
{
  if (m_vm)
    sq_close(m_vm);
}

bool SqVM::execute(const char* name)
{
  const Path path = m_cache.findFile(name);
  if (path.isEmpty())
  {
    logError("Failed to find script %s", name);
    return false;
  }

  std::ifstream stream(path.name());
  if (stream.fail())
  {
    logError("Failed to open script %s", name);
    return nullptr;
  }

  stream.seekg(0, std::ios::end);

  std::string text;
  text.resize((uint) stream.tellg());

  stream.seekg(0, std::ios::beg);
  stream.read(&text[0], text.size());
  stream.close();

  return execute(name, text.c_str());
}

bool SqVM::execute(const char* name, const char* text)
{
  if (SQ_FAILED(sq_compilebuffer(m_vm, text, std::strlen(text), name, true)))
    return false;

  sq_pushroottable(m_vm);

  const SQRESULT result = sq_call(m_vm, 1, false, true);

  sq_poptop(m_vm);
  return SQ_SUCCEEDED(result);
}

SqVM::operator HSQUIRRELVM ()
{
  return m_vm;
}

void* SqVM::foreignPointer() const
{
  return sq_getforeignptr(m_vm);
}

void SqVM::setForeignPointer(void* newValue)
{
  sq_setforeignptr(m_vm, newValue);
}

SqTable SqVM::rootTable()
{
  return SqTable::rootTable(m_vm);
}

SqTable SqVM::constTable()
{
  return SqTable::constTable(m_vm);
}

SqTable SqVM::registryTable()
{
  sq_pushregistrytable(m_vm);
  SqTable table(m_vm, -1);
  sq_poptop(m_vm);

  return table;
}

ResourceCache& SqVM::cache() const
{
  return m_cache;
}

void SqVM::onLogMessage(HSQUIRRELVM vm, const SQChar* format, ...)
{
  va_list vl;

  va_start(vl, format);
  std::string message = vlformat(format, vl);
  va_end(vl);

  log("%s", message.c_str());
}

void SqVM::onLogError(HSQUIRRELVM vm, const SQChar* format, ...)
{
  va_list vl;

  va_start(vl, format);
  std::string message = vlformat(format, vl);
  va_end(vl);

  logError("%s", message.c_str());
}

void SqVM::onCompilerError(HSQUIRRELVM vm,
                           const SQChar* description,
                           const SQChar* source,
                           SQInteger line,
                           SQInteger column)
{
  logError("%s:%i:%i: %s", source, int(line), int(column), description);
}

SQInteger SqVM::onRuntimeError(HSQUIRRELVM vm)
{
  if (sq_gettop(vm) >= 1)
    logErrorCallStack(vm);

  return 0;
}

SqObject::SqObject():
  m_vm(nullptr)
{
  sq_resetobject(&m_handle);
}

SqObject::SqObject(HSQUIRRELVM vm, SQInteger index):
  m_vm(vm)
{
  if (!m_vm)
    panic("VM handle cannot be NULL when constructing from stack");

  sq_resetobject(&m_handle);
  sq_getstackobj(m_vm, -1, &m_handle);
  sq_addref(m_vm, &m_handle);
}

SqObject::SqObject(const SqObject& source):
  m_vm(source.m_vm),
  m_handle(source.m_handle)
{
  sq_addref(m_vm, &m_handle);
}

SqObject::SqObject(SqObject&& source):
  m_vm(source.m_vm),
  m_handle(source.m_handle)
{
  source.m_vm = nullptr;
  sq_resetobject(&m_handle);
}

SqObject::~SqObject()
{
  if (m_vm)
    sq_release(m_vm, &m_handle);
}

SqObject SqObject::clone() const
{
  if (!m_vm)
    return SqObject();

  sq_pushobject(m_vm, m_handle);
  sq_clone(m_vm, -1);
  SqObject clone(m_vm, -1);
  sq_pop(m_vm, 2);
  return clone;
}

SqObject& SqObject::operator = (const SqObject& source)
{
  HSQOBJECT next = source.m_handle;
  sq_addref(m_vm, &next);
  sq_release(m_vm, &m_handle);
  m_handle = next;
  m_vm = source.m_vm;
  return *this;
}

std::string SqObject::asString() const
{
  if (!m_vm)
    return std::string();

  sq_pushobject(m_vm, m_handle);
  sq_tostring(m_vm, -1);

  std::string result = detail::get<std::string>(m_vm, -1);

  sq_pop(m_vm, 2);
  return result;
}

SQObjectType SqObject::type() const
{
  if (!m_vm)
    return OT_NULL;

  sq_pushobject(m_vm, m_handle);
  SQObjectType type = sq_gettype(m_vm, -1);
  sq_poptop(m_vm);
  return type;
}

SqObject::SqObject(HSQUIRRELVM vm):
  m_vm(vm)
{
  sq_resetobject(&m_handle);
}

bool SqObject::removeSlot(const char* name)
{
  if (isNull())
    return false;

  sq_pushobject(m_vm, m_handle);
  sq_pushstring(m_vm, name, -1);

  const SQRESULT result = sq_deleteslot(m_vm, -2, false);

  sq_poptop(m_vm);
  return SQ_SUCCEEDED(result);
}

bool SqObject::addFunction(const char* name,
                           void* pointer,
                           size_t pointerSize,
                           SQFUNCTION function,
                           bool staticMember)
{
  if (isNull())
    return false;

  sq_pushobject(m_vm, m_handle);
  sq_pushstring(m_vm, name, -1);

  std::memcpy(sq_newuserdata(m_vm, pointerSize), pointer, pointerSize);
  sq_newclosure(m_vm, function, 1);
  sq_setnativeclosurename(m_vm, -1, name);

  const SQRESULT result = sq_newslot(m_vm, -3, staticMember);

  sq_poptop(m_vm);
  return SQ_SUCCEEDED(result);
}

bool SqObject::clear()
{
  if (isNull())
    return false;

  sq_pushobject(m_vm, m_handle);

  const SQRESULT result = sq_clear(m_vm, -1);

  sq_poptop(m_vm);
  return SQ_SUCCEEDED(result);
}

SQInteger SqObject::size() const
{
  if (isNull())
    return 0;

  sq_pushobject(m_vm, m_handle);
  SQInteger size = sq_getsize(m_vm, -1);
  sq_poptop(m_vm);
  return size;
}

SqArray::SqArray(HSQUIRRELVM vm):
  SqObject(vm)
{
  sq_newarray(m_vm, 0);
  sq_getstackobj(m_vm, -1, &m_handle);
  sq_addref(m_vm, &m_handle);
  sq_poptop(m_vm);
}

SqArray::SqArray(const SqObject& source):
  SqObject(source)
{
  if (!isNull() && !isArray())
    panic("Object is not an array");
}

SqArray::SqArray(HSQUIRRELVM vm, SQInteger index):
  SqObject(vm, index)
{
  if (!isArray())
    panic("Object is not an array");
}

bool SqArray::remove(SQInteger index)
{
  if (isNull())
    return false;

  sq_pushobject(m_vm, m_handle);

  const SQRESULT result = sq_arrayremove(m_vm, -1, index);

  sq_poptop(m_vm);
  return SQ_SUCCEEDED(result);
}

bool SqArray::pop()
{
  if (isNull())
    return false;

  sq_pushobject(m_vm, m_handle);

  const SQRESULT result = sq_arraypop(m_vm, -1, false);

  sq_poptop(m_vm);
  return SQ_SUCCEEDED(result);
}

bool SqArray::resize(SQInteger newSize)
{
  if (isNull())
    return false;

  sq_pushobject(m_vm, m_handle);

  const SQRESULT result = sq_arrayresize(m_vm, -1, newSize);

  sq_poptop(m_vm);
  return SQ_SUCCEEDED(result);
}

bool SqArray::reverse()
{
  if (isNull())
    return false;

  sq_pushobject(m_vm, m_handle);

  const SQRESULT result = sq_arrayreverse(m_vm, -1);

  sq_poptop(m_vm);
  return SQ_SUCCEEDED(result);
}

SqObject SqArray::operator [] (SQInteger index) const
{
  if (isNull())
    panic("Cannot retrieve slot from null");

  sq_pushobject(m_vm, m_handle);
  sq_pushinteger(m_vm, index);

  if (SQ_FAILED(sq_get(m_vm, -2)))
  {
    sq_poptop(m_vm);
    panic("No array element at index");
  }

  SqObject result(m_vm, -1);
  sq_pop(m_vm, 2);
  return result;
}

SqTable::SqTable(HSQUIRRELVM vm):
  SqObject(vm)
{
  sq_newtable(m_vm);
  sq_getstackobj(m_vm, -1, &m_handle);
  sq_addref(m_vm, &m_handle);
  sq_poptop(m_vm);
}

SqTable::SqTable(const SqObject& source):
  SqObject(source)
{
  if (!isNull() && !isTable())
    panic("Object is not a table");
}

SqTable::SqTable(HSQUIRRELVM vm, SQInteger index):
  SqObject(vm, index)
{
  if (!isTable())
    panic("Object is not a table");
}

SqTable SqTable::rootTable(HSQUIRRELVM vm)
{
  sq_pushroottable(vm);
  SqTable table(vm, -1);
  sq_poptop(vm);

  return table;
}

SqTable SqTable::constTable(HSQUIRRELVM vm)
{
  sq_pushconsttable(vm);
  SqTable table(vm, -1);
  sq_poptop(vm);

  return table;
}

SqClass::SqClass(HSQUIRRELVM vm):
  SqObject(vm)
{
  sq_newclass(m_vm, false);
  sq_getstackobj(m_vm, -1, &m_handle);
  sq_addref(m_vm, &m_handle);
  sq_poptop(m_vm);
}

SqClass::SqClass(const SqObject& source):
  SqObject(source)
{
  if (!isNull() && !isClass())
    panic("Object is not a class");
}

SqClass::SqClass(HSQUIRRELVM vm, SQInteger index):
  SqObject(vm, index)
{
  if (!isClass())
    panic("Object is not a class");
}

SqInstance SqClass::createInstance() const
{
  if (isNull())
    panic("Cannot create instance of null");

  sq_pushobject(m_vm, m_handle);
  sq_createinstance(m_vm, -1);

  SqInstance result(m_vm, -1);

  sq_pop(m_vm, 2);
  return result;
}

SqTable SqClass::attributes()
{
  if (isNull())
    return SqTable();

  sq_pushobject(m_vm, m_handle);
  sq_pushnull(m_vm);
  sq_getattributes(m_vm, -2);

  SqTable result(m_vm, -1);

  sq_pop(m_vm, 2);
  return result;
}

SqTable SqClass::memberAttributes(const char* name)
{
  if (isNull())
    return SqTable();

  sq_pushobject(m_vm, m_handle);
  sq_pushstring(m_vm, name, -1);
  sq_getattributes(m_vm, -2);

  SqTable result(m_vm, -1);

  sq_pop(m_vm, 2);
  return result;
}

SqInstance::SqInstance(const SqObject& source):
  SqObject(source)
{
  if (!isNull() && !isInstance())
    panic("Object is not an instance");
}

SqInstance::SqInstance(HSQUIRRELVM vm, SQInteger index):
  SqObject(vm, index)
{
  if (!isInstance())
    panic("Object is not an instance");
}

SqClass SqInstance::class_() const
{
  sq_pushobject(m_vm, m_handle);
  sq_getclass(m_vm, -1);

  SqClass result(m_vm, -1);

  sq_pop(m_vm, 2);
  return result;
}

void* SqInstance::pointer()
{
  sq_pushobject(m_vm, m_handle);
  void* result = nullptr;
  sq_getinstanceup(m_vm, -1, &result, nullptr);
  sq_poptop(m_vm);
  return result;
}

} /*namespace wendy*/

