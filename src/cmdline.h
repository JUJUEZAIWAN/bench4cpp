
#ifndef CMDLINE_H
#define CMDLINE_H

#include <iostream>
#include <functional>
#include <vector>
#include <initializer_list>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <sstream>
#include <cstring>

namespace cmdpaser
{

    using namespace std;

    template <typename Target, typename Source, bool Same>
    class lexical_cast_t
    {
    public:
        static Target cast(const Source &arg)
        {
            Target ret;
            std::stringstream ss;
            if (!(ss << arg && ss >> ret && ss.eof()))
                throw std::bad_cast();

            return ret;
        }
    };

    template <typename Target, typename Source>
    class lexical_cast_t<Target, Source, true>
    {
    public:
        static Target cast(const Source &arg)
        {
            return arg;
        }
    };

    template <typename Source>
    class lexical_cast_t<std::string, Source, false>
    {
    public:
        static std::string cast(const Source &arg)
        {
            std::ostringstream ss;
            ss << arg;
            return ss.str();
        }
    };

    template <typename Target>
    class lexical_cast_t<Target, std::string, false>
    {
    public:
        static Target cast(const std::string &arg)
        {
            Target ret;
            std::istringstream ss(arg);
            if (!(ss >> ret && ss.eof()))
                throw std::bad_cast();
            return ret;
        }
    };
    template <>
    class lexical_cast_t<bool, std::string, false>
    {
    public:
        static bool cast(const std::string &arg)
        {
            bool ret;
            if(arg=="true"||arg=="TRUE"||arg=="True")
                ret=true;
            else if(arg=="false"||arg=="FALSE"||arg=="False")
                ret=false;
            else
                throw std::bad_cast();
            return ret;
        }
    };



    template <typename Target, typename Source>
    Target lexical_cast(const Source &arg)
    {
        return lexical_cast_t<Target, Source, is_same<Target, Source>::value>::cast(arg);
    }

    template <typename T>
    class oneof
    {
    public:
        oneof(initializer_list<T> &&vals) : choice(std::forward<initializer_list<T>>(vals))
        {
        }
        bool operator()(const T &val)
        {
            auto iter = choice.find(val);
            return iter != choice.end();
        }

    private:
        unordered_set<T> choice;
    };

    template <typename T>
    class notOneof
    {
    public:
        notOneof(initializer_list<T> &&vals) : choice(vals)
        {
        }
        bool operator()(const T &val)
        {
            auto iter = choice.find(val);
            return iter == choice.end();
        }

    private:
        unordered_set<T> choice;
    };

    template <typename T>
    class notRangeof
    {
    public:
        notRangeof(T &&low, T &&high) : low(low), high(high)
        {
        }
        bool operator()(const T &val)
        {
            return val < low || val > high;
        }

    private:
        T low, high;
    };

    template <typename T>
    class rangeof
    {
    public:
        rangeof(T &&low, T &&high) : low(low), high(high)
        {
        }
        bool operator()(const T &val)
        {
            return val >= low && val <= high;
        }

    private:
        T low, high;
    };

    class cmdline_error : public std::exception
    {
    public:
        cmdline_error(const std::string &msg) : msg(msg) {}
        ~cmdline_error() throw() {}
        const char *what() const throw() { return msg.c_str(); }

    private:
        std::string msg;
    };

    // option class
    class optionBase
    {
    public:
        optionBase(const string &name, char sname, const string &desc) : long_name(name), short_name(sname), descrition(desc)
        {
        }
        optionBase(const optionBase &) = default;
        virtual ~optionBase() = default;

        virtual const string &name() const { return long_name; }
        virtual char sname() const { return short_name; }
        virtual const string &desc() const { return descrition; }
        virtual void set(const string &val){};

    private:
        string long_name;
        char short_name;
        string descrition;
    };

    template <typename T>
    class option : public optionBase
    {
    public:
        option(const string &name, char sname, const string &desc) 
        : optionBase(name, sname, desc), is_default(false)
        {
        }

        option(const string &name, char sname, const string &desc, function<bool(T)> lim) 
        : optionBase(name, sname, desc), is_default(false), limiter(lim), exist_val(false)
        {
        }

        option(const string &name, char sname, const string &desc, const T &default_val, function<bool(T)> lim)
         : optionBase(name, sname, desc), is_default(true),default_value(default_val),  limiter(lim), exist_val(false)
        {
        }

        ~option() = default;

    public:
        const T &value() const
        {
            if (exist_val)
                return input_value;
            else if (is_default)
                return default_value;
            else
                throw cmdline_error(string("no value in option ") + name());
        }

        virtual void set(const string &val)
        {
            set_value(lexical_cast<decltype(input_value)>(val));
        }

    private:
        void set_value(const T &value)
        {
            if (limiter(value))
            {
                input_value = value;
                exist_val = true;
            }
            else
                throw cmdline_error("Input value does not meet limiter requirements");
        }

    private:
        bool is_default;
        T default_value;
        T input_value;
        function<bool(T)> limiter;
        bool exist_val;
        
        
        
    };

    class Parser
    {
    public:
        Parser() : sname_is_unique(true) {}
        Parser(bool unique_sname) : sname_is_unique(unique_sname) {}
        ~Parser() = default;

        void add(const string &name, char sname, const string &desc)
        {
            if (!options.count(name))
            {
                options[name] = make_shared<optionBase>(name, sname, desc);
            }
            add_name_sanme(name, sname);
        }

        template <typename T>
        void add(
            const string &name, char sname, const string &desc, function<bool(T)> limit = [](const T &)
                                                                { return true; })
        {
            if (!options.count(name))
            {
                options[name] = make_shared<option<T>>(name, sname, desc, limit);
            }
            add_name_sanme(name, sname);
        }

        template <typename T>
        void add(
            const string &name, char sname, const string &desc, const T &default_val, function<bool(T)> limit = [](const T &)
                                                                                      { return true; })
        {

            if (!limit(default_val))
                throw cmdline_error("Default value does not meet limiter requirements");
            if (!options.count(name))
            {

                options[name] = make_shared<option<T>>(name, sname, desc, default_val, limit);
            }
            add_name_sanme(name, sname);
        }

        template <typename T>
        const T &get(const string &name)
        {
            auto iter = options.find(name);
            if (iter != options.end())
            {
                return dynamic_pointer_cast<option<T>>(iter->second)->value();
            }
            else
            {
                throw cmdline_error("no " + name + " in options");
            }
        }

        const string &desc(const string &name) const
        {
            auto iter = options.find(name);
            if (iter != options.end())
            {
                return iter->second->desc();
            }
            else
            {
                throw cmdline_error("no " + name + " in options");
            }
        }

        char short_name(const string &name) const
        {
            auto iter = options.find(name);
            if (iter != options.end())
            {
                return iter->second->sname();
            }
            else
            {
                throw cmdline_error("no name: " + name + " in options");
            }
        }

        void set(const string &name, const string &value)
        {
            auto iter = options.find(name);
            if (iter != options.end())
                iter->second->set(value);
            else
                throw cmdline_error("no name: " + name + " in options");
        }

        void set(char sname, const string &value)
        {
            auto iter = nameMap.find(sname);
            if (iter != nameMap.end())
            {
                auto iter2 = options.find(iter->second);
                iter2->second->set(value);
            }
            else
                throw cmdline_error(string("no sname:") + sname + "in options");
        }

        void set_program_name(const string &p_name)
        {
            options["program"] = make_shared<optionBase>(p_name, 0, "program name");
        }

    public:
        bool parse(int argc, const char *const argv[])
        {
            if (argc < 1)
                return false;
            set_program_name(argv[0]);
            bool is_key = false, is_sname = false;
            string name, val;
            char sname;
            for (int i = 1; i < argc; i++)
            {
                if (strncmp(argv[i], "--", 2) == 0)
                {
                    is_key = true;
                    auto p = strchr(argv[i] + 2, '=');
                    if (p)
                    {
                        name = string(argv[i] + 2, p);
                        val = string(p + 1);
                        set(name, val);

                        is_key = false;
                        continue;
                    }
                    name = string(argv[i] + 2);
                }
                else if (strncmp(argv[i], "-", 1) == 0)
                {
                    is_key = true;
                    auto p = strchr(argv[i] + 1, '=');
                    if (p && argv[i] + 2 == p)
                    {
                        sname = *(argv[i] + 1);
                        val = string(p + 1);
                        set(sname, val);
                        is_key = false;
                        continue;
                    }
                    is_sname = true;
                    sname = *(argv[i] + 1);
                }

                if (is_key)
                {
                    if (i + 1 < argc && strncmp(argv[i + 1], "--", 2) != 0 && strncmp(argv[i + 1], "-", 1) != 0)
                    {
                        val = string(argv[i + 1]);

                        if (is_sname)
                        {
                            set(sname, val);
                            is_sname = false;
                        }
                        else
                            set(name, val);
                        is_key = false;

                        i = i + 1;
                    }
                    continue;
                }
                else
                    return false;
            }
            return true;
        }

    private:
        void add_name_sanme(const string &name, char sname)
        {
            if (!nameMap.count(sname))
            {
                nameMap[sname] = name;
            }
            else
            {
                if (!sname_is_unique)
                    nameMap[sname] = name;
                else
                    throw cmdline_error("short name already exists");
            }
        }

    private:
        unordered_map<string, shared_ptr<optionBase>> options;
        unordered_map<char, string> nameMap;
        bool sname_is_unique;
    };
} // namespace cmdpaser

#endif