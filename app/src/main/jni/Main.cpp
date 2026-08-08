#include <list>
#include <vector>
#include <cstring>
#include <pthread.h>
#include <thread>
#include <jni.h>
#include <unistd.h>
#include <dlfcn.h>
#include "Includes/Logger.h"
#include "Includes/obfuscate.h"
#include "Includes/Utils.hpp"
#include "Menu/Menu.hpp"
#include "Menu/Jni.hpp"
#include "Includes/Macros.h"
#include "dobby.h"

// Variáveis existentes
int scoreMul = 1, coinsMul = 1;
bool btnPressed = false;

// Variáveis para os novos Cheats de Armas
bool dDanoAlto = false;
bool dFireRate = false;
bool dMunicao = false;

// Hook no construtor da WeaponInfo (RVA: 0x16A6CF0)
void (*old_WeaponInfo_ctor)(void *instance);
void WeaponInfo_ctor(void *instance) {
    old_WeaponInfo_ctor(instance);
    if (instance != nullptr) {
        if (dDanoAlto) {
            *(int *) ((uintptr_t) instance + 0x1D0) = 9999; // _DamageQ0
            *(int *) ((uintptr_t) instance + 0x1D4) = 9999; // _DamageQ1
        }
        if (dFireRate) {
            *(float *) ((uintptr_t) instance + 0x5C) = 0.01f; // FireRate
        }
        if (dMunicao) {
            *(int *) ((uintptr_t) instance + 0x1F4) = 999; // _Magazine
        }
    }
}

jobjectArray GetFeatureList(JNIEnv *env, jobject context) {
    const char *features[] = {
            OBFUSCATE("Toggle_Dano Alto"),
            OBFUSCATE("Toggle_Tiro Rápido (Fire Rate)"),
            OBFUSCATE("Toggle_Munição Infinita"),
            OBFUSCATE("Toggle_No death"),
            OBFUSCATE("Button_Start Invcibility"),
            OBFUSCATE("SeekBar_Score multiplier_1_100"),
            OBFUSCATE("SeekBar_Coins multiplier_1_1000")
    };
    // ... (restante da lógica de array igual ao seu original)
    int Total_Feature = (sizeof features / sizeof features[0]);
    jobjectArray ret = env->NewObjectArray(Total_Feature, env->FindClass(OBFUSCATE("java/lang/String")), env->NewStringUTF(""));
    for (int i = 0; i < Total_Feature; i++) env->SetObjectArrayElement(ret, i, env->NewStringUTF(features[i]));
    return ret;
}

void Changes(JNIEnv *env, jclass clazz, jobject obj, jint featNum, jstring featName, jint value, jlong Lvalue, jboolean boolean, jstring text) {
    switch (featNum) {
        case 0: dDanoAlto = boolean; break;
        case 1: dFireRate = boolean; break;
        case 2: dMunicao = boolean; break;
        case 3: /* Caso do seu No Death original */ break;
        // ... (mantenha os outros cases originais ajustando os números conforme a nova ordem)
    }
}

// ... (mantenha suas funções Update, AddScore, AddCoins originais)

void hack_thread() {
    while (!isLibraryLoaded(targetLibName)) { sleep(1); }

#if defined(__aarch64__)
    // Novos Hooks
    HOOK(targetLibName, "0x16A6CF0", WeaponInfo_ctor, old_WeaponInfo_ctor);
    
    // Seus Hooks originais abaixo
    StartInvcibility = (void (*)(void *, float)) getAbsoluteAddress(targetLibName, OBFUSCATE("0x107A3BC"));
    HOOK(targetLibName, "0x107A2FC", AddCoins, old_AddCoins);
    install_hook_AddScore(getAbsoluteAddress(targetLibName, OBFUSCATE("0x107A2E0")));
    HOOK(targetLibName, "0x1078C44", Update, old_Update);
#endif
    LOGI(OBFUSCATE("Mod Menu Carregado com Sucesso!"));
}

__attribute__((constructor))
void lib_main() {
    std::thread(hack_thread).detach();
}
