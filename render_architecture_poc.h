/*
 * Proof of Concept: Render Architecture с Render Graph и мульти-бекендом
 * 
 * Иерархия:
 *   Engine::Run() → Renderer::Render() → RenderGraph::Execute() → Backend::Commands
 * 
 * Эта архитектура демонстрирует:
 *   1. Разделение ответственности между Renderer и RenderGraph
 *   2. Абстракция бекендов (Vulkan, OpenGL, DirectX)
 *   3. Декларативное описание рендеринга через Render Graph
 */

#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>
#include <algorithm>
#include <cstdint>

// ============================================================================
// ЧАСТЬ 1: АБСТРАКТНЫЙ СЛОЙ (общий для всех бекендов)
// ============================================================================

/**
 * @brief Абстрактный класс для команд рендеринга.
 * 
 * Это интерфейс, который реализуют все бекенды (Vulkan, OpenGL, DirectX).
 * High-level код работает только с этим интерфейсом.
 */
class CommandList {
public:
    virtual ~CommandList() = default;
    
    virtual void BeginRenderPass(const std::string& name, uint32_t width, uint32_t height) = 0;
    virtual void BindPipeline(const std::string& pipelineName) = 0;
    virtual void BindDescriptor(uint32_t set, uint32_t binding) = 0;
    virtual void Draw(uint32_t vertexCount, uint32_t instanceCount) = 0;
    virtual void EndRenderPass() = 0;
    
    // Для отладки
    virtual const char* GetBackendName() const = 0;
};

/**
 * @brief Абстрактный класс для ресурса (текстура, буфер).
 */
class Resource {
public:
    virtual ~Resource() = default;
    virtual const std::string& GetName() const = 0;
    virtual const char* GetType() const = 0;  // "Texture", "Buffer", etc.
};

/**
 * @brief Абстрактный класс для Render Pass.
 * 
 * Конкретные проходы (Geometry, Lighting, PostProcess) наследуются от этого класса.
 */
class RenderPass {
public:
    virtual ~RenderPass() = default;
    
    /**
     * @brief Выполнить проход рендеринга.
     * @param cmd Список команд для выполнения.
     * @param scene Данные сцены для рендеринга.
     */
    virtual void Execute(CommandList& cmd, const void* sceneData) = 0;
    
    /**
     * @brief Получить имя прохода.
     */
    virtual const std::string& GetName() const = 0;
    
    /**
     * @brief Получить список входных ресурсов (что читает этот pass).
     */
    virtual std::vector<std::string> GetInputs() const = 0;
    
    /**
     * @brief Получить список выходных ресурсов (что записывает этот pass).
     */
    virtual std::vector<std::string> GetOutputs() const = 0;
};

// ============================================================================
// ЧАСТЬ 2: КОНКРЕТНЫЕ БЕКЕНДЫ (Vulkan, OpenGL, DirectX)
// ============================================================================

/**
 * @brief Vulkan реализация CommandList.
 */
class VulkanCommandList : public CommandList {
public:
    VulkanCommandList() {
        std::cout << "  [Vulkan] CommandList created\n";
    }
    
    void BeginRenderPass(const std::string& name, uint32_t width, uint32_t height) override {
        std::cout << "  [Vulkan] vkCmdBeginRendering: " << name 
                  << " (" << width << "x" << height << ")\n";
        // В реальности: vkCmdBeginRendering(...)
    }
    
    void BindPipeline(const std::string& pipelineName) override {
        std::cout << "  [Vulkan] vkCmdBindPipeline: " << pipelineName << "\n";
        // В реальности: vkCmdBindPipeline(...)
    }
    
    void BindDescriptor(uint32_t set, uint32_t binding) override {
        std::cout << "  [Vulkan] vkCmdBindDescriptorSets: set=" 
                  << set << ", binding=" << binding << "\n";
        // В реальности: vkCmdBindDescriptorSets(...)
    }
    
    void Draw(uint32_t vertexCount, uint32_t instanceCount) override {
        std::cout << "  [Vulkan] vkCmdDraw: vertices=" << vertexCount 
                  << ", instances=" << instanceCount << "\n";
        // В реальности: vkCmdDraw(...)
    }
    
    void EndRenderPass() override {
        std::cout << "  [Vulkan] vkCmdEndRendering\n";
        // В реальности: vkCmdEndRendering(...)
    }
    
    const char* GetBackendName() const override {
        return "Vulkan";
    }
};

/**
 * @brief OpenGL реализация CommandList.
 */
class GLCommandList : public CommandList {
public:
    GLCommandList() {
        std::cout << "  [OpenGL] CommandList created\n";
    }
    
    void BeginRenderPass(const std::string& name, uint32_t width, uint32_t height) override {
        std::cout << "  [OpenGL] glBindFramebuffer + glViewport: " << name 
                  << " (" << width << "x" << height << ")\n";
        // В реальности: glBindFramebuffer(...), glViewport(...)
    }
    
    void BindPipeline(const std::string& pipelineName) override {
        std::cout << "  [OpenGL] glUseProgram: " << pipelineName << "\n";
        // В реальности: glUseProgram(...)
    }
    
    void BindDescriptor(uint32_t set, uint32_t binding) override {
        std::cout << "  [OpenGL] glBindTexture/Buffer: set=" 
                  << set << ", binding=" << binding << "\n";
        // В реальности: glBindTexture(...), glBindBuffer(...)
    }
    
    void Draw(uint32_t vertexCount, uint32_t instanceCount) override {
        std::cout << "  [OpenGL] glDrawArraysInstanced: vertices=" << vertexCount 
                  << ", instances=" << instanceCount << "\n";
        // В реальности: glDrawArraysInstanced(...)
    }
    
    void EndRenderPass() override {
        std::cout << "  [OpenGL] glEndFramebuffer\n";
        // В реальности: glBindFramebuffer(0, ...)
    }
    
    const char* GetBackendName() const override {
        return "OpenGL";
    }
};

/**
 * @brief DirectX 12 реализация CommandList.
 */
class DXCommandList : public CommandList {
public:
    DXCommandList() {
        std::cout << "  [DirectX12] CommandList created\n";
    }
    
    void BeginRenderPass(const std::string& name, uint32_t width, uint32_t height) override {
        std::cout << "  [DirectX12] OMSetRenderTargets: " << name 
                  << " (" << width << "x" << height << ")\n";
        // В реальности: ID3D12GraphicsCommandList::OMSetRenderTargets(...)
    }
    
    void BindPipeline(const std::string& pipelineName) override {
        std::cout << "  [DirectX12] SetPipelineState: " << pipelineName << "\n";
        // В реальности: ID3D12GraphicsCommandList::SetPipelineState(...)
    }
    
    void BindDescriptor(uint32_t set, uint32_t binding) override {
        std::cout << "  [DirectX12] SetGraphicsRootDescriptorTable: set=" 
                  << set << ", binding=" << binding << "\n";
        // В реальности: ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable(...)
    }
    
    void Draw(uint32_t vertexCount, uint32_t instanceCount) override {
        std::cout << "  [DirectX12] DrawInstanced: vertices=" << vertexCount 
                  << ", instances=" << instanceCount << "\n";
        // В реальности: ID3D12GraphicsCommandList::DrawInstanced(...)
    }
    
    void EndRenderPass() override {
        std::cout << "  [DirectX12] EndRenderPass\n";
        // В реальности: ID3D12GraphicsCommandList::EndRenderPass(...)
    }
    
    const char* GetBackendName() const override {
        return "DirectX12";
    }
};

// ============================================================================
// ЧАСТЬ 3: RENDER GRAPH (управляет порядком выполнения)
// ============================================================================

/**
 * @brief Render Graph — декларативное описание рендеринга как DAG.
 * 
 * Граф знает:
 *   - Какие ресурсы существуют (текстуры, буферы)
 *   - Какие проходы есть и их зависимости
 *   - В каком порядке выполнять проходы
 * 
 * Граф НЕ знает:
 *   - О сцене, камере, мешах
 *   - О конкретном API рендеринга
 */
class RenderGraph {
public:
    /**
     * @brief Добавить ресурс в граф.
     */
    void AddResource(const std::string& name, const std::string& type) {
        resources_[name] = type;
        std::cout << "  [RenderGraph] Added resource: " << name << " (" << type << ")\n";
    }
    
    /**
     * @brief Добавить проход рендеринга.
     */
    void AddPass(std::unique_ptr<RenderPass> pass) {
        std::cout << "  [RenderGraph] Added pass: " << pass->GetName() << "\n";
        passes_.push_back(std::move(pass));
        needsRecompile_ = true;
    }
    
    /**
     * @brief Скомпилировать граф (анализ зависимостей + топологическая сортировка).
     * 
     * Вызывается ОДИН РАЗ при инициализации или при изменении графа.
     */
    void Compile() {
        if (!needsRecompile_) {
            std::cout << "  [RenderGraph] Already compiled, skipping\n";
            return;
        }
        
        std::cout << "  [RenderGraph] Compiling graph...\n";
        
        // Топологическая сортировка на основе зависимостей
        executionOrder_.clear();
        std::unordered_map<std::string, size_t> resourceToPass;
        
        // Строим карту: какой pass производит какой ресурс
        for (size_t i = 0; i < passes_.size(); ++i) {
            for (const auto& output : passes_[i]->GetOutputs()) {
                resourceToPass[output] = i;
            }
        }
        
        // Простая топологическая сортировка (DFS)
        std::vector<bool> visited(passes_.size(), false);
        for (size_t i = 0; i < passes_.size(); ++i) {
            if (!visited[i]) {
                TopologicalSort(i, visited, resourceToPass);
            }
        }
        
        needsRecompile_ = false;
        std::cout << "  [RenderGraph] Compilation complete. Execution order:\n";
        for (size_t i : executionOrder_) {
            std::cout << "    " << (i + 1) << ". " << passes_[i]->GetName() << "\n";
        }
    }
    
    /**
     * @brief Выполнить граф.
     * 
     * Вызывается КАЖДЫЙ КАДР для рендеринга.
     */
    void Execute(CommandList& cmd, const void* sceneData) {
        if (needsRecompile_) {
            Compile();
        }
        
        std::cout << "\n  [RenderGraph] Executing " << executionOrder_.size() 
                  << " passes with " << cmd.GetBackendName() << " backend:\n";
        
        for (size_t i : executionOrder_) {
            std::cout << "  [RenderGraph] Executing pass: " << passes_[i]->GetName() << "\n";
            passes_[i]->Execute(cmd, sceneData);
        }
        
        std::cout << "  [RenderGraph] Execution complete\n";
    }
    
    /**
     * @brief Пометить граф как требующий пересборки.
     */
    void MarkDirty() {
        needsRecompile_ = true;
    }
    
private:
    struct Resource {
        std::string name;
        std::string type;
    };
    
    std::unordered_map<std::string, std::string> resources_;
    std::vector<std::unique_ptr<RenderPass>> passes_;
    std::vector<size_t> executionOrder_;
    bool needsRecompile_ = false;
    
    void TopologicalSort(size_t passIndex, std::vector<bool>& visited,
                         const std::unordered_map<std::string, size_t>& resourceToPass) {
        if (visited[passIndex]) return;
        visited[passIndex] = true;
        
        // Сначала обрабатываем зависимости (входы)
        for (const auto& input : passes_[passIndex]->GetInputs()) {
            auto it = resourceToPass.find(input);
            if (it != resourceToPass.end()) {
                TopologicalSort(it->second, visited, resourceToPass);
            }
        }
        
        executionOrder_.push_back(passIndex);
    }
};

// ============================================================================
// ЧАСТЬ 4: КОНКРЕТНЫЕ RENDER PASS (Geometry, Lighting, PostProcess)
// ============================================================================

// Данные сцены для рендеринга
struct SceneData {
    uint32_t entityCount = 0;
    uint32_t lightCount = 0;
    bool hasTransparency = false;
};

/**
 * @brief Geometry Pass — рендерит геометрию в G-Buffer.
 */
class GeometryPass : public RenderPass {
public:
    GeometryPass(uint32_t width, uint32_t height) 
        : width_(width), height_(height) {}
    
    void Execute(CommandList& cmd, const void* sceneData) override {
        auto* scene = static_cast<const SceneData*>(sceneData);
        
        cmd.BeginRenderPass("GeometryPass", width_, height_);
        cmd.BindPipeline("GeometryPipeline");
        cmd.BindDescriptor(0, 0);  // Camera UBO
        
        std::cout << "    [GeometryPass] Rendering " << scene->entityCount << " entities\n";
        cmd.Draw(3, scene->entityCount);  // 3 vertices per entity (triangles)
        
        cmd.EndRenderPass();
    }
    
    const std::string& GetName() const override {
        static const std::string name = "GeometryPass";
        return name;
    }
    
    std::vector<std::string> GetInputs() const override {
        return {};  // Geometry pass не читает другие ресурсы
    }
    
    std::vector<std::string> GetOutputs() const override {
        return {"GBuffer_Position", "GBuffer_Normal", "GBuffer_Albedo", "DepthBuffer"};
    }
    
private:
    uint32_t width_, height_;
};

/**
 * @brief Lighting Pass — рассчитывает освещение из G-Buffer.
 */
class LightingPass : public RenderPass {
public:
    LightingPass(uint32_t width, uint32_t height) 
        : width_(width), height_(height) {}
    
    void Execute(CommandList& cmd, const void* sceneData) override {
        auto* scene = static_cast<const SceneData*>(sceneData);
        
        cmd.BeginRenderPass("LightingPass", width_, height_);
        cmd.BindPipeline("LightingPipeline");
        cmd.BindDescriptor(0, 0);  // G-Buffer Position
        cmd.BindDescriptor(1, 1);  // G-Buffer Normal
        cmd.BindDescriptor(2, 2);  // G-Buffer Albedo
        cmd.BindDescriptor(3, 3);  // Light Buffer
        
        std::cout << "    [LightingPass] Computing lighting for " << scene->lightCount << " lights\n";
        cmd.Draw(3, 1);  // Full-screen quad
        
        cmd.EndRenderPass();
    }
    
    const std::string& GetName() const override {
        static const std::string name = "LightingPass";
        return name;
    }
    
    std::vector<std::string> GetInputs() const override {
        return {"GBuffer_Position", "GBuffer_Normal", "GBuffer_Albedo"};
    }
    
    std::vector<std::string> GetOutputs() const override {
        return {"LightingResult"};
    }
    
private:
    uint32_t width_, height_;
};

/**
 * @brief PostProcess Pass — применяет пост-эффекты.
 */
class PostProcessPass : public RenderPass {
public:
    PostProcessPass(uint32_t width, uint32_t height) 
        : width_(width), height_(height) {}
    
    void Execute(CommandList& cmd, const void* sceneData) override {
        auto* scene = static_cast<const SceneData*>(sceneData);
        
        cmd.BeginRenderPass("PostProcessPass", width_, height_);
        cmd.BindPipeline("PostProcessPipeline");
        cmd.BindDescriptor(0, 0);  // LightingResult
        cmd.BindDescriptor(1, 1);  // Tone mapping LUT
        
        std::cout << "    [PostProcessPass] Applying tone mapping, bloom, etc.\n";
        cmd.Draw(3, 1);  // Full-screen quad
        
        cmd.EndRenderPass();
    }
    
    const std::string& GetName() const override {
        static const std::string name = "PostProcessPass";
        return name;
    }
    
    std::vector<std::string> GetInputs() const override {
        return {"LightingResult"};
    }
    
    std::vector<std::string> GetOutputs() const override {
        return {"FinalImage"};
    }
    
private:
    uint32_t width_, height_;
};

// ============================================================================
// ЧАСТЬ 5: RENDERER (высокоуровневая логика рендеринга)
// ============================================================================

/**
 * @brief Renderer — знает ЧТО рендерить.
 * 
 * Renderer отвечает за:
 *   - Отсечение невидимых объектов (culling)
 *   - Обновление uniform-буферов
 *   - Настройку RenderGraph для текущего кадра
 *   - Выбор бекенда для выполнения
 */
class Renderer {
public:
    Renderer() {
        std::cout << "[Renderer] Creating renderer\n";
        
        // Инициализация RenderGraph (один раз при старте)
        InitRenderGraph();
    }
    
    /**
     * @brief Установить бекенд.
     */
    void SetBackend(std::unique_ptr<CommandList> backend) {
        backend_ = std::move(backend);
        std::cout << "[Renderer] Backend set to: " << backend_->GetBackendName() << "\n";
    }
    
    /**
     * @brief Рендерить сцену.
     * 
     * Это основной метод рендеринга, вызываемый каждый кадр.
     */
    void Render(const SceneData& scene) {
        std::cout << "\n[Renderer] Rendering scene with " << scene.entityCount 
                  << " entities and " << scene.lightCount << " lights\n";
        
        // 1. Culling (определение видимых объектов)
        auto visibleEntities = CullScene(scene);
        std::cout << "[Renderer] After culling: " << visibleEntities << " visible entities\n";
        
        // 2. Обновление данных для графа
        currentSceneData_ = scene;
        
        // 3. Выполнение RenderGraph
        if (backend_) {
            renderGraph_.Execute(*backend_, &currentSceneData_);
        } else {
            std::cerr << "[Renderer] ERROR: No backend set!\n";
        }
        
        std::cout << "[Renderer] Render complete\n";
    }
    
private:
    std::unique_ptr<CommandList> backend_;
    RenderGraph renderGraph_;
    SceneData currentSceneData_;
    
    void InitRenderGraph() {
        std::cout << "[Renderer] Initializing RenderGraph\n";
        
        // Регистрация ресурсов
        renderGraph_.AddResource("GBuffer_Position", "Texture");
        renderGraph_.AddResource("GBuffer_Normal", "Texture");
        renderGraph_.AddResource("GBuffer_Albedo", "Texture");
        renderGraph_.AddResource("DepthBuffer", "DepthStencil");
        renderGraph_.AddResource("LightingResult", "Texture");
        renderGraph_.AddResource("FinalImage", "Texture");
        
        // Добавление проходов
        renderGraph_.AddPass(std::make_unique<GeometryPass>(1920, 1080));
        renderGraph_.AddPass(std::make_unique<LightingPass>(1920, 1080));
        renderGraph_.AddPass(std::make_unique<PostProcessPass>(1920, 1080));
        
        // Компиляция графа (один раз при инициализации)
        renderGraph_.Compile();
    }
    
    uint32_t CullScene(const SceneData& scene) {
        // Простая заглушка для culling
        // В реальности здесь была бы проверка frustum + occlusion culling
        return scene.entityCount;  // Для примера считаем всё видимым
    }
};

// ============================================================================
// ЧАСТЬ 6: ENGINE (главный класс приложения)
// ============================================================================

/**
 * @brief Engine — главный класс приложения.
 * 
 * Управляет:
 *   - Игровым циклом
 *   - Сценой
 *   - Renderer
 */
class Engine {
public:
    /**
     * @brief Инициализировать движок.
     */
    bool Initialize(const std::string& backendName) {
        std::cout << "===========================================\n";
        std::cout << "Engine::Initialize()\n";
        std::cout << "===========================================\n";
        
        // Создание бекенда
        if (backendName == "Vulkan") {
            renderer_.SetBackend(std::make_unique<VulkanCommandList>());
        } else if (backendName == "OpenGL") {
            renderer_.SetBackend(std::make_unique<GLCommandList>());
        } else if (backendName == "DirectX") {
            renderer_.SetBackend(std::make_unique<DXCommandList>());
        } else {
            std::cerr << "Unknown backend: " << backendName << "\n";
            return false;
        }
        
        running_ = true;
        return true;
    }
    
    /**
     * @brief Главный игровой цикл.
     */
    void Run() {
        std::cout << "\n===========================================\n";
        std::cout << "Engine::Run() - Main Loop\n";
        std::cout << "===========================================\n";
        
        uint32_t frameCount = 0;
        const uint32_t maxFrames = 3;  // Для примера всего 3 кадра
        
        while (running_ && frameCount < maxFrames) {
            frameCount++;
            std::cout << "\n>>> FRAME " << frameCount << " <<<\n";
            
            // 1. Update (логика игры)
            Update();
            
            // 2. Render (рендеринг)
            Render();
        }
        
        std::cout << "\n===========================================\n";
        std::cout << "Engine::Run() - Loop finished after " 
                  << frameCount << " frames\n";
        std::cout << "===========================================\n";
    }
    
    /**
     * @brief Остановить движок.
     */
    void Stop() {
        running_ = false;
    }
    
private:
    Renderer renderer_;
    bool running_ = false;
    SceneData scene_;
    
    void Update() {
        // Обновление логики игры
        scene_.entityCount = 100;  // Для примера
        scene_.lightCount = 5;
        std::cout << "[Engine::Update] Scene updated\n";
    }
    
    void Render() {
        // Вызов рендерера
        renderer_.Render(scene_);
    }
};

// ============================================================================
// ЧАСТЬ 7: ТОЧКА ВХОДА (демонстрация)
// ============================================================================

/**
 * @brief Точка входа для демонстрации архитектуры.
 */
inline void RunRenderArchitecturePoC() {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║  Proof of Concept: Render Architecture                   ║\n";
    std::cout << "║  Engine → Renderer → RenderGraph → Backend               ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";
    std::cout << "\n";
    
    // Демонстрация с разными бекендами
    std::vector<std::string> backends = {"Vulkan", "OpenGL", "DirectX"};
    
    for (const auto& backend : backends) {
        std::cout << "\n";
        std::cout << "═══════════════════════════════════════════════════════\n";
        std::cout << "  TESTING WITH " << backend << " BACKEND\n";
        std::cout << "═══════════════════════════════════════════════════════\n";
        
        Engine engine;
        if (engine.Initialize(backend)) {
            engine.Run();
        }
    }
    
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║  Proof of Concept Complete!                              ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";
    std::cout << "\n";
}
