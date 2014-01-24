//
//  event.h
//  uv
//
//  Created by Christopher Tarquini on 10/10/13.
//  Copyright (c) 2013 Christopher Tarquini. All rights reserved.
//

#ifndef uv_event_h
#define uv_event_h


#include <functional>
#include <list>
#include <iterator>
#include <map>
#include <typeinfo>



template <typename... Arguments>
class Event {
    
public:
    typedef void (*HandlerFn)(Arguments...);
    typedef std::function<void(Arguments...)> Handler;
    typedef std::vector<Handler> HandlerList;
    typedef typename HandlerList::iterator Binding;
    typedef Binding (*addFn)(Handler);
    typedef void (*emitFn)(Arguments...);
    typedef typename HandlerList::iterator iterator;
    
    iterator addListener(Handler fn){
        listeners_.push_back(fn);
        return listeners_.back();
    };
    
    iterator on(Handler fn){
        return addListener(fn);
    };
    
    iterator once(Handler fn){
        iterator* ptr = new iterator;
        *ptr = addListener([this, ptr](Arguments... args){
            this->removeListener(*ptr);
            delete ptr;
            fn(std::forward<Arguments>(args)...);
        });
    };
    
    void removeListener(iterator binding) {
        listeners_.erase(binding);
    };
    
    void off(iterator binding){
        removeListener(binding);
    };
    
    void removeAllListeners() {
        listeners_.clear();
    };
    void off(){
        removeAllListeners();
    }
    
    HandlerList listeners(){
        return listeners_;
    }
    
    size_t size(){
        return listeners_.size();
    }
    
    
    void emit(Arguments&&... args) {
        for(auto fn = listeners_.begin(), end = listeners_.end(); fn != end; fn++){
            (*fn)(std::forward<Arguments>(args)...);
        }
    }
    void operator()(Arguments&&... args){
        emit(std::forward<Arguments>(args)...);
    };
    
    iterator operator()(Handler fn){
        return addListener(fn);
    }
    
private:
    static const unsigned int argsize = sizeof...(Arguments);
    unsigned int max_listeners_ = 10;
    HandlerList listeners_;
    
};





#endif
