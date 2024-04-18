#include <napi.h>
#include <string>
#include <unordered_map>

extern "C" {
    #include "../../../src/main.h"
}

Napi::Object TSP_runner(const Napi::CallbackInfo& info){
    Napi::Env env = info.Env();

    std::string temp = (std::string) info[0].ToString();
    const char* filename = temp.data();
    int seed = (int) info[1].As<Napi::Number>();
    int time_limit = (int) info[2].As<Napi::Number>();
    int alg = (int) info[3].As<Napi::Number>();
    return_struct* rs = webapp_run(filename, seed, time_limit, alg);

    // javascript object
    Napi::Object ret = Napi::Object::New(env);
    ret.Set("cost", Napi::Number::New(env, (double)rs->cost));

    // array of points
    Napi::Array points = Napi::Array::New(info.Env(), rs->nnodes);
    for(int i=0; i<rs->nnodes; i++){
        Napi::Object obj = Napi::Object::New(env);
        obj.Set("x", rs->points[i].x);
        obj.Set("y", rs->points[i].y);
        points[i] = obj;
    }

    ret.Set("points", points);

    // array for links (called path)
    Napi::Array path = Napi::Array::New(info.Env(), rs->nnodes);
    for(int i=0; i<rs->nnodes; i++){
        path[i] = rs->path[i];
    }

    ret.Set("path", path);

    ret.Set("nnodes", rs->nnodes);

    return ret;
}

Napi::Object Init(Napi::Env env, Napi::Object exports){
    exports.Set(
        Napi::String::New(env, "TSP_runner"),
        Napi::Function::New(env, TSP_runner)
    );

    return exports;
}

NODE_API_MODULE(TSP, Init);
