
#pragma once

#include <string>
#include <map>
#include "..\libcc\libcc\stringutil.hpp"
#include "ddemirc.h"

// RAII critsec class
class CriticalSection
{
public:
  CriticalSection() { InitializeCriticalSection(&m_cs); }
  ~CriticalSection() { DeleteCriticalSection(&m_cs); }
  bool Enter() { EnterCriticalSection(&m_cs); return true; }
  bool Leave() { LeaveCriticalSection(&m_cs); return true; }
  bool TryEnter() { return TryEnterCriticalSection(&m_cs) != 0; }// returns true if we entered it OK.

  class ScopeLock
  {
  public:
    ScopeLock(CriticalSection& x) : m_cs(x) { m_cs.Enter(); }
    ~ScopeLock() { m_cs.Leave(); }
  private:
    CriticalSection& m_cs;
  };
private:
  CRITICAL_SECTION m_cs;
};

namespace mirc
{
  class MediaInfoWrapper
  {
  public:
    MediaInfoWrapper(IWMPMedia* p) :
      m_p(p)
    {
    }

    std::wstring GetAttribute(const std::wstring& name)
    {
      CComBSTR bstrVal;
      std::wstring out(L"n/a");
      if(SUCCEEDED(m_p->getItemInfo(CComBSTR(name.c_str()), &bstrVal)))
      {
        if(bstrVal.Length())
        {
          out = bstrVal;
          //if(name == L"Duration")
          //{
          //  // format time.
          //  DWORD f = wcstoul(out.c_str(), 0, 10);
          //  DWORD seconds = f % 60;
          //  DWORD minutes = (f - seconds) / 60;
          //  out = LibCC::FormatW("%:%").ul(minutes).ul<10,2>(seconds).Str();
          //}
        }
      }
      return out;
    }

  private:
    IWMPMedia* m_p;
  };

  /*
    Terminology:
    command string:   blah$(date;b%cd;aoeu)
    variable:         date;b%cd;aoeu
    attribute name:   aoeu
    attribute type:   date
    attribute format: b%cd
    field:            cd
    field name:       c
    field format:     d
    attribute name:   aoeu

    $(attr) = grab the text from the attribute
    $(type;format;attr) = grabs the text, analyzes it, and displays it in a custom format

    format:
      General usage:
      %ab where a=field, b=format
      b:
        x=default
        t=textual representation
        n=n-digit
        1=1-digit, 0-prefix
        ...
        9=9-digit, 0-prefix

    $(sec;;attr) = treat the attribute like a number in seconds, and format it like mm:ss
    $(date;;attr) = treat the attribute like number of bits, and display in kbps
    $(bits;%k;attr) = treat the attribute like number of bytes, and display in kb
    $(s;;attr) = treat the attribute like number of bytes, and display in mb
  */
  enum VariableType
  {
    // date
    VT_DATE,
    // duration
    VT_MILLISECONDS,
    VT_SECONDS,
    VT_MINUTES,
    VT_HOURS,
    VT_DAYS,
    // size
    VT_BITS,
    VT_BYTES,
    VT_KILOBITS,
    VT_KILOBYTES,
    VT_MEGABYTES,
    VT_MEGABITS,
    // filename
    VT_FILENAME
  };
  inline std::wstring VariableTypeToString(VariableType type)
  {
    switch(type)
    {
    case VT_DATE:
      return L"date";
    case VT_MILLISECONDS:
      return L"milliseconds";
    case VT_SECONDS:
      return L"seconds";
    case VT_MINUTES:
      return L"mintutes";
    case VT_HOURS:
      return L"hours";
    case VT_DAYS:
      return L"days";
    case VT_BITS:
      return L"bits";
    case VT_BYTES:
      return L"bytes";
    case VT_KILOBITS:
      return L"kilobits";
    case VT_KILOBYTES:
      return L"kilobytes";
    case VT_MEGABITS:
      return L"megabits";
    case VT_MEGABYTES:
      return L"megabytes";
    case VT_FILENAME:
      return L"filename";
    }
    return L"unknown";
  }
  inline bool StringToVariableType(const std::wstring& str, VariableType& out)
  {
    std::map<std::wstring, VariableType> themap;
    themap[L"d"] = VT_DATE;
    themap[L"dt"] = VT_DATE;
    themap[L"date"] = VT_DATE;
    themap[L"time"] = VT_DATE;
    themap[L"datetime"] = VT_DATE;
    themap[L"t"] = VT_BITS;
    themap[L"bit"] = VT_BITS;
    themap[L"bits"] = VT_BITS;
    themap[L"b"] = VT_BYTES;
    themap[L"byte"] = VT_BYTES;
    themap[L"bytes"] = VT_BYTES;
    themap[L"kbit"] = VT_KILOBITS;
    themap[L"kbits"] = VT_KILOBITS;
    themap[L"kilobit"] = VT_KILOBITS;
    themap[L"kilobits"] = VT_KILOBITS;
    themap[L"kb"] = VT_KILOBYTES;
    themap[L"kbyte"] = VT_KILOBYTES;
    themap[L"kbytes"] = VT_KILOBYTES;
    themap[L"kilobyte"] = VT_KILOBYTES;
    themap[L"kilobytes"] = VT_KILOBYTES;
    themap[L"mb"] = VT_MEGABYTES;
    themap[L"mbyte"] = VT_MEGABYTES;
    themap[L"mbytes"] = VT_MEGABYTES;
    themap[L"megabyte"] = VT_MEGABYTES;
    themap[L"megabytes"] = VT_MEGABYTES;
    themap[L"mbit"] = VT_MEGABITS;
    themap[L"mbits"] = VT_MEGABITS;
    themap[L"megabit"] = VT_MEGABITS;
    themap[L"megabits"] = VT_MEGABITS;
    themap[L"s"] = VT_SECONDS;
    themap[L"sec"] = VT_SECONDS;
    themap[L"secs"] = VT_SECONDS;
    themap[L"second"] = VT_SECONDS;
    themap[L"seconds"] = VT_SECONDS;
    themap[L"ms"] = VT_MILLISECONDS;
    themap[L"millisecond"] = VT_MILLISECONDS;
    themap[L"milliseconds"] = VT_MILLISECONDS;
    themap[L"f"] = VT_FILENAME;
    themap[L"file"] = VT_FILENAME;
    themap[L"filename"] = VT_FILENAME;
    std::wstring lwr = LibCC::StringToLower(str);
    std::map<std::wstring, VariableType>::iterator it = themap.find(lwr);
    if(it == themap.end())
    {
      return false;
    }
    out = it->second;
    return true;
  }

  //enum FieldName
  //{
  //  // datetime
  //  FN_YEAR,       // Y
  //  FN_MONTH,      // M
  //  FN_DAY,        // D - duration
  //  FN_24HOUR,     // H
  //  FN_12HOUR,     // h - duration
  //  FN_AMPM,       // p
  //  FN_MINUTE,     // n - duration
  //  FN_SECOND,     // s - duration
  //  FN_MILLISECOND,// m - duration
  //  // size
  //  FN_BITS,       // b
  //  FN_BYTES,      // B
  //  FN_KILOBITS,   // k
  //  FN_KILOBYTES,  // K
  //  FN_MEGABYTES,  // m
  //  FN_MEGABITS ,  // M
  //  // filename
  //  FN_EXTENSION,  // e
  //  FN_PATH,       // p
  //  FN_FILENAMEWITHEXTENSION,// f
  //  FN_FILENAMENOEXTENSION   // n
  //};

  enum FieldFormat
  {
    FF_NDIGIT,
    FF_TEXT,
    FF_DEFAULT,
    FF_FIXEDDIGIT
  };

  inline std::wstring ConvertOneDigitNumberToText(unsigned __int64 x)
  {
    static PCWSTR omg[] = { L"zero", L"one", L"two", L"three", L"four", L"five", L"six", L"seven", L"eight", L"nine" };
    return omg[x % 10];
  }

  inline std::wstring ConvertTensDigitToText(unsigned __int64 x)
  {
    static PCWSTR omg[] = { L"", L"", L"twenty", L"thirty", L"forty", L"fifty", L"sixty", L"seventy", L"eighty", L"ninety" };
    return omg[x / 10];
  }

  inline std::wstring ConvertTwoDigitNumberToText(unsigned __int64 x)
  {
    // x is always below 100
    if(x < 9) return ConvertOneDigitNumberToText(x);
    if(x < 20)
    {
      static PCWSTR teens[] = { L"ten", L"eleven", L"twelve", L"thirteen", L"fourteen", L"fifteen", L"sixteen", L"seventeen", L"eighteen", L"nineteen" };
      return teens[x-10];
    }
    if(0 == (x % 10))// on a 10-digit boundary
    {
      return ConvertTensDigitToText(x);
    }
    // your typical 2-digit number like fifty-one
    return LibCC::Format("%-%").s(ConvertTensDigitToText(x)).s(ConvertOneDigitNumberToText(x)).Str();
  }

  inline std::wstring ConvertThreeDigitNumberToText(unsigned __int64 x)
  {
    // x is always below 1,000
    if(x < 100) return ConvertTwoDigitNumberToText(x);
    if(0 == (x % 100))// on a hundred boundary
    {
      return LibCC::Format("% hundred").s(ConvertOneDigitNumberToText(x / 100)).Str();
    }
    return LibCC::Format("% hundred %").s(ConvertOneDigitNumberToText(x / 100)).s(ConvertTwoDigitNumberToText(x % 100)).Str();
  }

  inline std::wstring ConvertNumberToText(unsigned __int64 n)
  {
    //     4 four
    //    54 fifty-four
    //   654 six hundred fifty-four
    //  1654 one thousand six hundred fifty-four
    // 21654 twenty-one thousand six hundred fifty-four
    // convert the last 3 digits & tack on a suffix (thousands, etc) and then shift.
    // 18,446,744,073,709,551,615 <-- largest number
    std::wstring ret;
    static PCWSTR suffixes[] = { L"", L" thousand", L" million", L" billion", L" trillion", L" quadrillion", L" quintillion" };
    for(int i = 0; ; i ++)
    {
      std::wstring temp = ConvertThreeDigitNumberToText(n % 1000);
      n = n / 1000;
      temp.append(suffixes[i]);
      if(ret.size()) { temp.push_back(' '); }
      ret = temp + ret;
      if(n == 0) break;
    }
    return ret;
  }

  inline std::wstring ProcessNumericField(unsigned __int64 n, wchar_t fieldFormat)
  {
    switch(fieldFormat)
    {
    case 't':
      // convert the number to text.
      return ConvertNumberToText(n);
    case 'x':
    case 'n':
      return LibCC::Format().ui64(n).Str();
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      return LibCC::Format().ui64(n, 10, fieldFormat - '0').Str();
    }
    return L"Unknown numeric field format";
  }

  inline std::wstring ProcessDurationField(unsigned __int64 ms, wchar_t fieldName, wchar_t fieldFormat)
  {
    unsigned __int64 converted;
    switch(fieldName)
    {
    case 'm':
      // ms OF second.
      converted = ms % 1000;
      break;
    case 'M':
      converted = ms;
      break;
    case 's':
      // second of minute
      converted = ms / 1000;
      converted %= 60;
      break;
    case 'S':
      converted = ms / 1000;
      break;
    case 'n':
      // minute of hour
      converted = ms / 1000 / 60;
      converted %= 60;
      break;
    case 'N':
      converted = ms / 1000 / 60;
      break;
    case 'h':
      // hour of day
      converted = ms / 1000 / 60 / 60;
      converted %= 24;
      break;
    case 'H':
      converted = ms / 1000 / 60 / 60;
      break;
    case 'd':
    case 'D':
      converted = ms / 1000 / 60 / 60 / 24;
      break;
    default:
      return L"Unknown duration field name";
    }
    return ProcessNumericField(converted, fieldFormat);
  }
  inline std::wstring ProcessSizeField(unsigned __int64 bits, wchar_t fieldName, wchar_t fieldFormat)
  {
    unsigned __int64 converted;

    switch(fieldName)
    {
    case 'b':
      converted = bits;
      break;
    case 'B':
      converted = bits / 8;
      break;
    case 'k':
      converted = bits / 8 / 128;
      break;
    case 'K':
      converted = bits / 8 / 128 / 8;
      break;
    case 'm':
      converted = bits / 8 / 128 / 8 / 128;
      break;
    case 'M':
      converted = bits / 8 / 128 / 8 / 128 / 8;
      break;
    default:
      return L"Unknown size field name";
    }
    // now format it based on fieldformat.
    return ProcessNumericField(converted, fieldFormat);
  }

  inline std::wstring ProcessFileNameField(const std::wstring& url, wchar_t fieldName)
  {
    switch(fieldName)
    {
    case 'e':
      {
        std::wstring ret = PathFindExtensionW(url.c_str());
        if(ret.size())
        {
          if(ret[0] == '.')
          {
            ret = ret.substr(1);// skip the dot.  i don't want dots.
          }
        }
        return ret;
      }
    case 'p':
      {
        // path (without filename or slash)
        PCWSTR szURL = url.c_str();
        PCWSTR szFileName = PathFindFileNameW(szURL);
        std::wstring ret = url.substr(0, szFileName - szURL);
        // remove backslash.
        if(ret.size())
        {
          if(*(ret.rbegin()) == '\\')
          {
            ret.resize(ret.size() - 1);
          }
        }
        return ret;
      }
    case 'f':
      {
        return PathFindFileNameW(url.c_str());
      }
    case 'n':
      {
        std::wstring ret = PathFindFileNameW(url.c_str());
        // remove extension
        PCWSTR szURL = ret.c_str();
        PCWSTR szExt = PathFindExtensionW(szURL);
        if(ret.size())
        {
          ret.resize(szExt - szURL);
        }
        return ret;
      }
    }
    return L"[Unknown filename field]";
  }

  inline std::wstring ProcessField(MediaInfoWrapper& mi, VariableType& type, const wchar_t* field/*[2]*/, const std::wstring& name)
  {
    // for the description, [field, format]
    std::wstring value = mi.GetAttribute(name);
    wchar_t fieldFormat = field[1];
    wchar_t fieldName = field[0];

    unsigned __int64 size = wcstoul(value.c_str(), 0, 10);

    // convert to one of the common types (date, bits, milliseconds, filename) and dispatch
    switch(type)
    {
    //// date
    //case VT_DATE:
    //  {
    //    // get the date.
    //    CComVariant v(value.c_str());
    //    v.ChangeType(VT_DATE);
    //    int a = 0;
    //  }
    //  break;

    // duration
    case VT_SECONDS:
      size *= 1000;
    case VT_MILLISECONDS:
      {
        return ProcessDurationField(size, fieldName, fieldFormat);
      }
    // size - convert to bits.
    case VT_MEGABYTES:
      size *= 8;
    case VT_MEGABITS:
      size *= 128;
    case VT_KILOBYTES:
      size *= 8;
    case VT_KILOBITS:
      size *= 128;
    case VT_BYTES:
      size *= 8;
    case VT_BITS:
      {
        return ProcessSizeField(size, fieldName, fieldFormat);
      }
      break;
    // filename
    case VT_FILENAME:
      {
        return ProcessFileNameField(value, fieldName);
      }
      break;
    }
    return L"[Unrecognized field spec]";
  }

  inline std::wstring ProcessVariable(MediaInfoWrapper& mi, VariableType& type, const std::wstring& format, const std::wstring& name)
  {
    std::wstring out;

    // parse the format string.
    for(std::wstring::const_iterator it = format.begin(); it != format.end(); ++ it)
    {
      switch(*it)
      {
      case '\\':
        ++ it;
        if(it == format.end()) break;
        out.push_back(*it);
        break;
      case '%':
        // get the field
        wchar_t field[2];
        ++ it;
        if(it == format.end()) break;
        field[0] = *it;
        ++ it;
        if(it == format.end()) break;
        field[1] = *it;
        out.append(ProcessField(mi, type, field, name));
        break;
      default:
        out.push_back(*it);
        break;
      }
      if(it == format.end()) break;
    }
    return out;
  }

  inline std::wstring ProcessVariable(MediaInfoWrapper& mi, const std::wstring& variable)
  {
    std::wstring currentArg(L"");
    std::vector<std::wstring> args;

    for(std::wstring::const_iterator it = variable.begin(); it != variable.end(); ++ it)
    {
      switch(*it)
      {
      case '\\':
        ++ it;
        if(it == variable.end()) break;
        currentArg.push_back(*it);
        break;
      case ';':
        // delimiter.
        args.push_back(currentArg);
        currentArg.clear();
        break;
      default:
        currentArg.push_back(*it);
        break;
      }
      if(it == variable.end()) break;
    }
    args.push_back(currentArg);
    currentArg.clear();

    switch(args.size())
    {
    case 1:
      return mi.GetAttribute(args.front());
    case 3:
      break;
    default:
      // error: wrong number of arguments
      return L"Wrong number of arguments.  Make sure the variable is in $(a;b;c) or $(c) format.";
    }

    // we know there are 3 arguments.
    VariableType type;
    if(!StringToVariableType(args[0], type))
    {
      return L"Unrecognized variable type.";
    }
    return ProcessVariable(mi, type, args[1], args[2]);
  }

  inline std::wstring ProcessCommandString(MediaInfoWrapper& mi, const std::wstring& fmt)
  {
    std::wstring out;
    for(std::wstring::const_iterator it = fmt.begin(); it != fmt.end(); ++ it)
    {
      switch(*it)
      {
      case '\\':
        ++ it;
        if(it == fmt.end()) break;
        out.push_back(*it);
        break;
      case '$':
        {
          ++ it;
          if(it == fmt.end()) break;
          if(*it != '(')
          {
            out.push_back('$');
            out.push_back(*it);
          }
          else
          {
            // we do have $(.  now find the next )
            std::wstring variable;
            for(++ it; it != fmt.end(); ++ it)// skip the (
            {
              if(*it == '\\')
              {
                ++ it;
                if(it == fmt.end()) break;
                if(*it == ')')// here i really only care about escaping close parens.  other escaping is handled in the next function downw.
                {
                  variable.push_back(*it);
                }
                else
                {
                  variable.push_back('\\');
                  variable.push_back(*it);
                }
              }
              else if(*it == ')')
              {
                out.append(ProcessVariable(mi, variable));
                break;
              }
              else
              {
                variable.push_back(*it);
              }
            }
            if(it == fmt.end())
            {
              out.append(variable);// we never found a ), so just plop it in there
              break;
            }
          }
        }
        break;
      default:
        out.push_back(*it);
        break;
      }

      if(it == fmt.end()) break;
    }

    return out;
  }

  inline void SendMircCommand(const std::wstring& cmd)
  {
    CallDDE(L"mIRC", cmd.c_str());
  }



}