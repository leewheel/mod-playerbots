/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_AIOBJECTCONTEXT_H
#define PLAYERBOTS_AIOBJECTCONTEXT_H

#include "Common.h"
#include "DynamicObject.h"
#include "NamedObjectContext.h"
#include "PlayerbotAIAware.h"
#include "Strategy.h"
#include "Trigger.h"
#include "Value.h"
#include <sstream>
#include <string>

class PlayerbotAI;

typedef Strategy* (*StrategyCreator)(PlayerbotAI* botAI);
typedef Action* (*ActionCreator)(PlayerbotAI* botAI);
typedef Trigger* (*TriggerCreator)(PlayerbotAI* botAI);
typedef UntypedValue* (*ValueCreator)(PlayerbotAI* botAI);

class AiObjectContext : public PlayerbotAIAware
{
public:
    static BoolCalculatedValue* custom_glyphs(PlayerbotAI* ai); // Added for cutom glyphs
    AiObjectContext(PlayerbotAI* botAI,
                    SharedNamedObjectContextList<Strategy>& sharedStrategyContext = sharedStrategyContexts,
                    SharedNamedObjectContextList<Action>& sharedActionContext = sharedActionContexts,
                    SharedNamedObjectContextList<Trigger>& sharedTriggerContext = sharedTriggerContexts,
                    SharedNamedObjectContextList<UntypedValue>& sharedValueContext = sharedValueContexts);
    virtual ~AiObjectContext() {}

    virtual Strategy* GetStrategy(std::string const name);
    virtual std::set<std::string> GetSiblingStrategy(std::string const name);
    virtual Trigger* GetTrigger(std::string const name);
    virtual Action* GetAction(std::string const name);
    virtual UntypedValue* GetUntypedValue(std::string const name);

    template <class T>
    Value<T>* GetValue(std::string const name)
    {
        return dynamic_cast<Value<T>*>(GetUntypedValue(name));
    }

    template <class T>
    Value<T>* GetValue(std::string const name, std::string const param)
    {
        // By leewheel 2026-08-19
        // 性能优化：原实现每次构造临时 std::string(name) + "::" + param（堆分配）。
        // 该函数是 AI_VALUE2 宏的核心路径，每 tick 每个 trigger 都要调用大量次数，
        // 字符串堆分配是主要开销之一。
        // 改为线程局部缓冲区复用拼接，消除堆分配；GetContextObject 内部以 name 为键
        // 做 map 查找（拷贝 key），不保留引用，因此缓冲区复用是安全的。
        thread_local std::string key;
        key.clear();
        key.reserve(name.size() + 2 + param.size());
        key += name;
        key += "::";
        key += param;
        return GetValue<T>(key);
        // End By leewheel
    }

    template <class T>
    Value<T>* GetValue(std::string const name, int32 param)
    {
        std::ostringstream out;
        out << param;
        return GetValue<T>(name, out.str());
    }

    std::set<std::string> GetValues();
    std::set<std::string> GetSupportedStrategies();
    std::set<std::string> GetSupportedActions();
    std::string const FormatValues();

    std::vector<std::string> Save();
    void Load(std::vector<std::string> data);

    std::vector<std::string> performanceStack;

    static void BuildAllSharedContexts();

    static void BuildSharedContexts();
    static void BuildSharedStrategyContexts(SharedNamedObjectContextList<Strategy>& strategyContexts);
    static void BuildSharedActionContexts(SharedNamedObjectContextList<Action>& actionContexts);
    static void BuildSharedTriggerContexts(SharedNamedObjectContextList<Trigger>& triggerContexts);
    static void BuildSharedValueContexts(SharedNamedObjectContextList<UntypedValue>& valueContexts);

protected:
    NamedObjectContextList<Strategy> strategyContexts;
    NamedObjectContextList<Action> actionContexts;
    NamedObjectContextList<Trigger> triggerContexts;
    NamedObjectContextList<UntypedValue> valueContexts;

private:
    static SharedNamedObjectContextList<Strategy> sharedStrategyContexts;
    static SharedNamedObjectContextList<Action> sharedActionContexts;
    static SharedNamedObjectContextList<Trigger> sharedTriggerContexts;
    static SharedNamedObjectContextList<UntypedValue> sharedValueContexts;
};

#endif
