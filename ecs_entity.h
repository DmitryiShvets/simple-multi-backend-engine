
//=================================================================================
// ТИПО ООП СТИЛЬ
// ================================================================================

// === ecs/entity.h ===
#pragma once
#include <flecs.h>

class ComponentBase {
protected:
  flecs::entity m_entity;

public:
  explicit ComponentBase(flecs::entity e) : m_entity(e) {}
  flecs::entity handle() const { return m_entity; }
};

class Entity {
protected:
  flecs::entity m_entity;

public:
  explicit Entity(flecs::entity e) : m_entity(e) {}

  // Component access с ООП стилем
  template<typename T>
  T& get() {
    return *m_entity.get<T>();
  }

  template<typename T>
  const T& get() const {
    return *m_entity.get<T>();
  }

  template<typename T>
  bool has() const {
    return m_entity.has<T>();
  }

  template<typename T, typename... Args>
  T& add(Args&&... args) {
    m_entity.add<T>();
    auto* comp = m_entity.get_mut<T>();
    new (comp) T(std::forward<Args>(args)...);
    return *comp;
  }

  template<typename T>
  void remove() {
    m_entity.remove<T>();
  }

  flecs::entity handle() const { return m_entity; }
};

// === Специализированные компоненты ===
class TransformComponent : public ComponentBase {
public:
  using ComponentBase::ComponentBase;

  glm::vec3 getPosition() const {
    auto& t = get<Transform>();
    return t.position;
  }

  void setPosition(const glm::vec3& pos) {
    auto& t = get<Transform>();
    t.position = pos;
    t.dirty = true;
  }

  glm::mat4 getModelMatrix() {
    auto& t = get<Transform>();
    if (t.dirty) {
      // Вычисляем матрицу
      t.modelMatrix = computeMatrix();
      t.dirty = false;
    }
    return t.modelMatrix;
  }

private:
  Transform& get() { return *m_entity.get<Transform>(); }
  const Transform& get() const { return *m_entity.get<Transform>(); }
};

class MeshComponent : public ComponentBase {
public:
  using ComponentBase::ComponentBase;

  void draw() const {
    auto& mesh = get<Mesh>();
    // Vulkan draw calls...
  }

private:
  Mesh& get() { return *m_entity.get<Mesh>(); }
};

// === World wrapper ===
class World {
private:
  flecs::world m_world;

public:
  Entity createEntity(const std::string& name = "") {
    auto e = m_world.entity();
    if (!name.empty()) {
      e.set_name(name.c_str());
    }
    return Entity(e);
  }

  // Query builder с ООП стилем
  template<typename... Components>
  auto query() {
    return m_world.query<Components...>();
  }

  // Системы
  template<typename... Components>
  class SystemBuilder {
    flecs::world& m_world;
  public:
    SystemBuilder(flecs::world& w) : m_world(w) {}

    SystemBuilder& each(std::function<void(Components&...)> callback) {
      m_world.system<Components...>()
        .each([callback](flecs::iter&, Components&... comps) {
          callback(comps...);
        });
      return *this;
    }
  };

  auto system() {
    return SystemBuilder<>{m_world};
  }

  flecs::world& handle() { return m_world; }
};

// === Использование (ООП стиль!) ===
class Game {
private:
  World m_world;

public:
  void init() {
    // Создание сущности
    auto player = m_world.createEntity("Player");
    player.add<Transform>();
    player.add<Mesh>();

    // Система с ООП callback
    m_world.system<Transform, Mesh>()
      .each([this](Transform& t, Mesh& m) {
        if (t.dirty) updateTransform(t);
        drawMesh(m, t.modelMatrix);
      });
  }

  void update() {
    // Query с ООП итерацией
    m_world.query<Transform, Mesh>()
      .each([](Transform& t, Mesh& m) {
        t.position += glm::vec3(0.01f, 0, 0);
        t.dirty = true;
      });
  }
};

//=================================================================================
// РАБОТАЮЩИЙ ГИБРИД
// ================================================================================


#pragma once
#include <flecs.h>

// ECS компоненты (данные)
struct Transform {
  glm::vec3 position{0};
  glm::vec3 rotation{0};
  glm::mat4 modelMatrix{1};
  bool dirty{true};
};

struct Mesh {
  VkBuffer vertexBuffer;
  VkBuffer indexBuffer;
  uint32_t indexCount;
};

// === Обёртки для ООП доступа (НЕ для query!) ===
class Entity {
private:
  flecs::entity m_entity;

public:
  explicit Entity(flecs::entity e) : m_entity(e) {}

  // Доступ к ECS компонентам
  template<typename T>
  T& get() { return *m_entity.get<T>(); }

  template<typename T>
  const T& get() const { return *m_entity.get<T>(); }

  template<typename T, typename... Args>
  Entity& add(Args&&... args) {
    m_entity.set<T>(std::forward<Args>(args)...);
    return *this;
  }

  // === ООП методы-обёртки ===
  glm::vec3 getPosition() const {
    auto& t = get<Transform>();
    return t.position;
  }

  void setPosition(const glm::vec3& pos) {
    auto& t = get<Transform>();
    t.position = pos;
    t.dirty = true;
  }

  glm::mat4 getModelMatrix() {
    auto& t = get<Transform>();
    if (t.dirty) {
      t.modelMatrix = computeMatrix(t);
      t.dirty = false;
    }
    return t.modelMatrix;
  }

  flecs::entity handle() const { return m_entity; }
};

// === World wrapper ===
class World {
private:
  flecs::world m_world;

public:
  Entity createEntity(const std::string& name = "") {
    auto e = m_world.entity();
    if (!name.empty()) e.set_name(name.c_str());
    return Entity(e);
  }

  template<typename... Components>
  auto query() {
    return m_world.query<Components...>();
  }

  flecs::world& handle() { return m_world; }
};
// === Использование ===

// ✅ Query к ECS данным (быстро, для систем)
world.query<Transform, Mesh>()
  .each([](Transform& t, Mesh& m) {
    if (t.dirty) t.modelMatrix = computeMatrix(t);
    drawMesh(m, t.modelMatrix);
  });

// ✅ ООП доступ через Entity wrapper (удобно, для логики)
auto player = world.createEntity("Player");
player.setPosition({1, 2, 3});  // ООП метод
auto& transform = player.get<Transform>();  // Прямой доступ к ECS

//=================================================================================
// ПРАВИЛЬНЫЙ ГИБРИД
//
// === Entity wrapper для создания и ООП доступа ===
// ================================================================================

class Entity {
public:
  void setPosition(const glm::vec3& pos) {
    get<Transform>().position = pos;
    get<Transform>().dirty = true;
  }

  glm::vec3 getPosition() const {
    return get<Transform>().position;
  }
};

// === Системы для массовой обработки ===
class TransformSystem {
public:
  static void update(flecs::world& world) {
    // Массовое обновление ВСЕХ трансформов
    world.query<Transform>()
      .each([](Transform& t) {
        if (t.dirty) {
          t.modelMatrix = computeMatrix(t);
          t.dirty = false;
        }
      });
  }
};

class RenderSystem {
public:
  static void render(flecs::world& world, Renderer& renderer) {
    // Массовый рендер ВСЕХ мешей
    world.query<const Mesh, const Transform>()
      .each([&](const Mesh& m, const Transform& t) {
        renderer.draw(m, t.modelMatrix);
      });
  }
};

// === Game loop ===
void update() {
  // ООП: изменение отдельных entity
  player.setPosition({1, 2, 3});
  enemy.setPosition({4, 5, 6});

  // ECS: массовая обработка
  TransformSystem::update(world);
  RenderSystem::render(world, renderer);
}


//=================================================================================
// Нужна ли обёртка для каждого компонента?
// Делай обёртки **только для тех компонентов, с которыми удобно работать через ОО Entity wrapper ===
// ================================================================================

//
class Entity {
private:
  flecs::entity m_entity;

public:
  // === Частые компоненты — ООП обёртки ===
  glm::vec3 getPosition() const {
    return get<Transform>().position;
  }

  void setPosition(const glm::vec3& pos) {
    auto& t = get<Transform>();
    t.position = pos;
    t.dirty = true;
  }

  glm::vec3 getVelocity() const {
    return get<Velocity>().value;
  }

  void setVelocity(const glm::vec3& vel) {
    get<Velocity>().value = vel;
  }

  bool isActive() const {
    return has<Active>() && get<Active>().value;
  }

  void setActive(bool active) {
    if (has<Active>()) {
      get<Active>().value = active;
    } else {
      add<Active>(active);
    }
  }

  // === Редкие компоненты — прямой доступ ===
  template<typename T>
  T& get() { return *m_entity.get<T>(); }

  template<typename T>
  bool has() const { return m_entity.has<T>(); }
};

//Компонент | Обёртка? | Почему |
// |-----------|---------|--------|
// | **Transform** | ✅ Да | Используем каждый кадр |
// | **Velocity** | ✅ Да | Частое изменение |
// | **Health** | ✅ Да | Игровая логика |
// | **Inventory** | ✅ Да | Сложная логика |
// | **MeshRenderer** | ❌ Нет | Только в render системе |
// | **AudioSource** | ❌ Нет | Только в audio системе |
// | **ParticleEmitter** | ❌ Нет | Только в particle системе


//=================================================================================
// Игорова логика в ООП стиле, либо с ООП обвертками ecs
// ВАРИАНТ 1: Без систем (логика в обёртках)
// ================================================================================

// === Игровая логика в обёртках ===
class Player {
  Entity m_entity;

public:
  void update(float dt) {
    auto& pos = m_entity.get<Transform>().position;
    auto& vel = m_entity.get<Velocity>().value;
    auto& health = m_entity.get<Health>().current;

    // Логика движения
    pos += vel * dt;

    // Логика здоровья
    if (health <= 0) {
      die();
    }
  }

  void takeDamage(float amount) {
    auto& health = m_entity.get<Health>().current;
    health -= amount;
  }

  void die() {
    // Смерть игрока
  }
};

// === Game loop ===
void Game::update(float dt) {
  player->update(dt);      // ❌ Каждый entity отдельно
  enemy1->update(dt);
  enemy2->update(dt);
  // ...

  // Нет массовой обработки!
}
//Проблемы:**
// - ❌ Нет кэш-локальности (прыгаем по памяти)
// - ❌ Каждый entity — отдельный вызов
// - ❌ Сложно добавить новые entity
// - ❌ Нет автоматического параллелизма


//=================================================================================
// Игорова логика в ecs стиле
//  ВАРИАНТ 2: Логика в системах (правильно)
// ================================================================================

// === Компоненты — только данные ===
struct Transform {
  glm::vec3 position;
  glm::vec3 velocity;
  bool dirty;
};

struct Health {
  float current;
  float max;
  bool dead;
};

struct PlayerTag {};  // Маркер
struct EnemyTag {};   // Маркер

// === Системы с логикой ===
class MovementSystem {
public:
  static void update(flecs::world& world, float dt) {
    // ВСЕ сущности с Transform обновляются за раз
    world.query<Transform>()
      .each([dt](Transform& t) {
        t.position += t.velocity * dt;
        t.dirty = true;
      });
  }
};

class HealthSystem {
public:
  static void update(flecs::world& world) {
    // ВСЕ сущности с Health обновляются за раз
    world.query<Health>()
      .each([](Health& h) {
        if (h.current <= 0) {
          h.dead = true;
        }
      });
  }
};

class PlayerSystem {
public:
  static void update(flecs::world& world, float dt) {
    // ТОЛЬКО игроки (есть PlayerTag)
    world.query<PlayerTag, Transform, Health>()
      .each([dt](Transform& t, Health& h) {
        // Специфичная логика игрока
        if (h.dead) {
          respawn(t);
        }
      });
  }
};

class EnemySystem {
public:
  static void update(flecs::world& world, float dt) {
    // ТОЛЬКО враги (есть EnemyTag)
    world.query<EnemyTag, Transform, Health>()
      .each([dt](Transform& t, Health& h) {
        // Специфичная логика врага
        aiLogic(t, h);
      });
  }
};

// === Game loop ===
void Game::update(float dt) {
  // ✅ Массовая обработка по системам
  MovementSystem::update(world, dt);   // Все entity
  HealthSystem::update(world);         // Все с health
  PlayerSystem::update(world, dt);     // Только игроки
  EnemySystem::update(world, dt);      // Только враги
}

//Преимущества:**
// - ✅ Кэш-локальность (линейный проход)
// - ✅ Массовая обработка
// - ✅ Фильтрация по тегам (PlayerTag, EnemyTag)
// - ✅ Автоматический параллелизм (Flecs может параллелить)

//=================================================================================
// ГИБРИДНЫЙ ПОДХОД (лучшее из обоих)
// ================================================================================

//
// === Entity wrapper для удобного API ===
class Entity {
public:
  // Частые операции — обёртки
  void takeDamage(float amount) {
    auto& h = get<Health>();
    h.current -= amount;
    if (h.current <= 0) {
      h.dead = true;
    }
  }

  void heal(float amount) {
    auto& h = get<Health>();
    h.current = std::min(h.current + amount, h.max);
  }

  void applyForce(const glm::vec3& force) {
    auto& v = get<Velocity>();
    v.value += force;
  }

  // Прямой доступ для систем
  template<typename T>
  T& get() { return *m_entity.get<T>(); }
};

// === Игровая логика ===
void Game::update(float dt) {
  // ✅ Точечные изменения через обёртки
  player.takeDamage(10);
  player.heal(5);
  player.applyForce({0, 9.8f, 0});

  // ✅ Массовая обработка через системы
  MovementSystem::update(world, dt);
  HealthSystem::update(world);
}
//=================================================================================
//  ПРАКТИЧЕСКИЙ ПРИМЕР
// Твой случай (Vulkan рендеринг):
// ================================================================================
// === Компоненты ===
struct Transform {
  glm::vec3 position;
  glm::mat4 modelMatrix;
  bool dirty;
};

struct Mesh {
  VkBuffer vertexBuffer;
  VkBuffer indexBuffer;
  uint32_t indexCount;
};

struct Renderable {
  VkDescriptorSet descriptorSet;
  bool visible;
};

// === Entity wrapper ===
class Entity {
public:
  void setPosition(const glm::vec3& pos) {
    auto& t = get<Transform>();
    t.position = pos;
    t.dirty = true;
  }

  void setMesh(VkBuffer vb, VkBuffer ib, uint32_t count) {
    m_entity.set<Mesh>(Mesh{vb, ib, count});
  }

  void setVisible(bool visible) {
    if (has<Renderable>()) {
      get<Renderable>().visible = visible;
    } else {
      m_entity.add<Renderable>(Renderable{VK_NULL_HANDLE, visible});
    }
  }

  template<typename T>
  T& get() { return *m_entity.get<T>(); }

  template<typename T>
  bool has() const { return m_entity.has<T>(); }
};

// === Render система ===
class RenderSystem {
public:
  static void render(flecs::world& world, Renderer& renderer) {
    // ТОЛЬКО видимые меши
    world.query<const Mesh, const Transform, const Renderable>()
      .each([&](const Mesh& m, const Transform& t, const Renderable& r) {
        if (r.visible) {
          renderer.draw(m, t.modelMatrix, r.descriptorSet);
        }
      });
  }
};

// === Использование ===
void Game::init() {
  auto player = world.createEntity("Player");
  player.setPosition({0, 0, 0});
  player.setMesh(vertexBuf, indexBuf, 1000);
  player.setVisible(true);
}

void Game::update(float dt) {
  // Точечные изменения
  player.setPosition({1, 2, 3});

  // Массовая обработка
  TransformSystem::update(world, dt);
  RenderSystem::render(world, renderer);
}

//Вопрос | Ответ |
// |--------|-------|
// | **Нужна ли обёртка для каждого компонента?** | ❌ Нет, только для частых |
// | **Логика только в системах?** | ✅ Да, для массовой обработки |
// | **Можно ли точечно менять?** | ✅ Да, через Entity wrapper |
// | **Что лучше?** | Гибрид: wrapper + системы
//

//=================================================================================
//  ХЕЛПЕРЫ
// ================================================================================

// === 1. Entity wrapper — базовые операции ===
class Entity {
public:
  // Только самое частое
  glm::vec3 getPosition() const;
  void setPosition(const glm::vec3&);
  float getHealth() const;
  void takeDamage(float);

  // Прямой доступ для остального
  template<typename T>
  T& get();
};

// === 2. Отдельные helper-классы для сложной логики ===
class AnimationHelper {
public:
  static void play(Entity& e, const std::string& anim, float blend) {
    auto& a = e.get<Animation>();
    a.current = anim;
    a.blendTime = blend;
  }
};

class AudioHelper {
public:
  static void setAttenuation(Entity& e, float min, float max) {
    auto& audio = e.get<AudioSource>();
    audio.minDistance = min;
    audio.maxDistance = max;
  }
};

// === 3. Системы для массовой обработки ===
class AnimationSystem {
public:
  static void update(flecs::world& world, float dt) {
    world.query<Animation, Transform>()
      .each([dt](Animation& a, Transform& t) {
        // Массовое обновление анимаций
      });
  }
};

// === Использование ===
void Game::update(float dt) {
  // Через Entity wrapper (часто)
  player.setPosition({1, 2, 3});
  player.takeDamage(10);

  // Через helper (редко, специфично)
  AnimationHelper::play(player, "attack", 0.3f);
  AudioHelper::setAttenuation(player, 1.0f, 50.0f);

  // Через системы (массовая обработка)
  AnimationSystem::update(world, dt);
  MovementSystem::update(world, dt);
}
