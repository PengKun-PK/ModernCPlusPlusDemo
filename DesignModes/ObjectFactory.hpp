#pragma once

#include <any>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "Singleton.hpp"

class ObjectFactory : public Singleton<ObjectFactory>
{
    friend class Singleton<ObjectFactory>;

public:
    template<typename T, typename... Args>
    T* create(Args&&... args)
    {
        return createImpl<T>(typeid(T).name(), std::forward<Args>(args)...);
    }

    template<typename T, typename... Args>
    T* createByName(const std::string& typeName, Args&&... args)
    {
        return createImpl<T>(typeName, std::forward<Args>(args)...);
    }

    template<typename T>
    void registerType(const std::string& typeName)
    {
        m_typeInfo[typeName] = TypeInfo{
            std::type_index(typeid(T)), [this](std::any args) -> std::unique_ptr<void, Deleter>
            { return std::unique_ptr<void, Deleter>(new T(std::any_cast<decltype(args)>(args)), Deleter{this}); },
            [](void* ptr) { delete static_cast<T*>(ptr); }};
    }

    template<typename T>
    std::vector<T*> getObjects() const
    {
        std::vector<T*> result;
        for (const auto& pair : m_objects)
        {
            if (pair.second.typeIndex == std::type_index(typeid(T)))
            {
                result.push_back(static_cast<T*>(pair.second.ptr.get()));
            }
        }
        return result;
    }

    void deleteObject(void* ptr)
    {
        auto it = std::find_if(m_objects.begin(), m_objects.end(),
                               [ptr](const auto& pair) { return pair.second.ptr.get() == ptr; });
        if (it != m_objects.end())
        {
            m_objects.erase(it);
        }
    }

    ~ObjectFactory()
    {
        std::cout << "ObjectFactory destructor: Cleaning up all objects." << std::endl;
        m_objects.clear();
    }

private:
    struct Deleter
    {
        ObjectFactory* factory;

        void operator()(void* ptr) const
        {
            factory->deleteObject(ptr);
        }
    };

    struct ObjectInfo
    {
        std::unique_ptr<void, Deleter> ptr;
        std::type_index typeIndex;
        std::function<void(void*)> deleter;
    };

    struct TypeInfo
    {
        std::type_index typeIndex;
        std::function<std::unique_ptr<void, Deleter>(std::any)> creator;
        std::function<void(void*)> deleter;
    };

    std::unordered_map<std::string, TypeInfo> m_typeInfo;
    std::unordered_map<void*, ObjectInfo> m_objects;

    template<typename T, typename... Args>
    T* createImpl(const std::string& typeName, Args&&... args)
    {
        auto it = m_typeInfo.find(typeName);
        if (it == m_typeInfo.end())
        {
            throw std::runtime_error("Unknown type: " + typeName);
        }

        auto any_args = std::make_any<decltype(std::make_tuple(std::forward<Args>(args)...))>(
            std::make_tuple(std::forward<Args>(args)...));
        auto unique_ptr = it->second.creator(any_args);
        T* ptr = static_cast<T*>(unique_ptr.get());
        m_objects[ptr] = ObjectInfo{std::move(unique_ptr), std::type_index(typeid(T)), it->second.deleter};
        return ptr;
    }

    ObjectFactory() = default;
};

//===========================Usage example=========================================
// // main.cpp
// #include "ObjectFactory.hpp"
// #include <iostream>
// #include <memory>

// // Base class for animals
// class Animal
// {
// public:
//     virtual ~Animal() = default;
//     virtual void makeSound() const = 0;
// };

// // Dog class
// class Dog : public Animal
// {
// public:
//     Dog(const std::string& name) : m_name(name) {}
//     void makeSound() const override
//     {
//         std::cout << m_name << " says: Woof!" << std::endl;
//     }
// private:
//     std::string m_name;
// };

// // Cat class
// class Cat : public Animal
// {
// public:
//     Cat(const std::string& name, int lives) : m_name(name), m_lives(lives) {}
//     void makeSound() const override
//     {
//         std::cout << m_name << " says: Meow! (Lives left: " << m_lives << ")" << std::endl;
//     }
// private:
//     std::string m_name;
//     int m_lives;
// };

// int main()
// {
//     // Get the singleton instance of ObjectFactory
//     ObjectFactory& factory = ObjectFactory::Instance();

//     // Register types
//     factory.registerType<Dog>("Dog");
//     factory.registerType<Cat>("Cat");

//     try
//     {
//         // Create objects
//         Dog* dog = factory.createByName<Dog>("Dog", "Buddy");
//         Cat* cat = factory.createByName<Cat>("Cat", "Whiskers", 9);

//         // Use the objects
//         dog->makeSound();
//         cat->makeSound();

//         // Get all animals and make them sound
//         std::cout << "\nAll animals:" << std::endl;
//         for (Animal* animal : factory.getObjects<Animal>())
//         {
//             animal->makeSound();
//         }

//         // Try to create an unregistered type
//         factory.createByName<Animal>("Bird", "Tweety");
//     }
//     catch(const std::exception& e)
//     {
//         std::cerr << "Error: " << e.what() << std::endl;
//     }

//     // The factory will automatically delete the objects when it's destroyed

//     return 0;
// }
