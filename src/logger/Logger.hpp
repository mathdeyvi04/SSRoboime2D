#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <atomic>
#include <format>

namespace fs = std::filesystem;

/**
 * @brief Singleton para logging assíncrono.
 * @details Focada em performance utiliza uma lógica de fila de mensagens.
 */
class Logger {
private:
    // Buffers para técnica de Double Buffering
    std::vector<std::string> m_current_buffer;
    std::vector<std::string> m_write_buffer;

    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::thread m_worker;
    std::atomic<bool> m_is_running;
    std::atomic<bool> m_is_the_first = true;
    std::ofstream m_file_stream;

public:
    /**
     * @brief Acesso à instância única
     */
    static Logger& get(){ static Logger instance; return instance; }

    Logger(const Logger&) = delete;
    void operator=(const Logger&) = delete;

    /**
     * @brief Adiciona log nível INFO.
     * @param msg Mensagem a ser imprimida.
     * @details Recebe por valor para permitir std::move (otimização de r-values).
     */
    void
    info(std::string msg){ log("[INFO] ", std::move(msg)); }

    /**
     * @brief Adiciona log nível WARN.
     * @param msg Mensagem a ser imprimida.
     * @details Recebe por valor para permitir std::move (otimização de r-values).
     */
    void
    warn(std::string msg){ log("[WARN] ", std::move(msg)); }

    /**
     * @brief Adiciona log nível ERROR.
     * @param msg Mensagem a ser imprimida.
     * @details Recebe por valor para permitir std::move (otimização de r-values).
     */
    void
    error(std::string msg){ log("[ERROR] ", std::move(msg)); }

    /**
     * @brief Log INFO usando C++20 std::format (Alta Performance).
     * @param fmt A string de formatação (ex: "Valor: {}"). Deve ser uma string literal (constante).
     * @param args Os argumentos a serem formatados.
     */
    template<typename... Args>
    void info(std::format_string<Args...> fmt, Args&&... args) {
        // std::format gera a std::string final de forma otimizada.
        // std::forward garante que não haja cópias desnecessárias dos argumentos.
        log("[INFO] ", std::format(fmt, std::forward<Args>(args)...));
    }

    /**
     * @brief Log WARN usando C++20 std::format.
     */
    template<typename... Args>
    void warn(std::format_string<Args...> fmt, Args&&... args) {
        log("[WARN] ", std::format(fmt, std::forward<Args>(args)...));
    }

    /**
     * @brief Log ERROR usando C++20 std::format.
     */
    template<typename... Args>
    void error(std::format_string<Args...> fmt, Args&&... args) {
        log("[ERROR] ", std::format(fmt, std::forward<Args>(args)...));
    }

private:
    /**
     * @brief Construtor privado: Inicializa arquivo e thread.
     * @details Reservará 1000 slots para evitarmos realocações
     */
    Logger() : m_is_running(true) {
        // Reserva memória prévia para evitar realocações frequentes no vetor
        m_current_buffer.reserve(30);
        m_write_buffer.reserve(30);
    }

    /**
     * @brief Destrutor: Sinaliza parada e espera thread terminar.
     */
    ~Logger(){
        m_is_running = false;
        m_cv.notify_one();  ///< Informa a thread da condição de encerramento

        if(m_worker.joinable()){ m_worker.join(); }
        if(m_file_stream.is_open()){ m_file_stream.close(); }
    }

    /**
     * @brief Responsável por criar ambiente de .log
     * @details Possui uma lógica para garantir que logs sejam únicos.
     */
    void
    init_file(){
        if(!fs::exists("logs")){ fs::create_directory("logs"); }

        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

        std::stringstream ss;
        ss << "logs/" << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d_%H-%M-%S") << ".log";

        // std::ios::app não é necessário se o arquivo é único por execução
        // mas útil se reiniciarmos o logger no mesmo segundo -> Impossível?
        m_file_stream.open(ss.str(), std::ios::out | std::ios::app);

        // Desabilita sincronização automática com stdio para performance
        std::ios_base::sync_with_stdio(false);
    }

    /**
     * @brief Responsável por providenciar genérica chamada de impressão em .log
     * @param prefixo Cabeçalho que será colocado antes da mensagem.
     * @param msg Mensagem principal.
     * Usa lock apenas para empurrar no vetor (operação de nanossegundos).
     */
    void
    log(const char* prefixo, std::string&& msg) {

        // --- INÍCIO DA ADIÇÃO DO TIMESTAMP ---
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

        std::stringstream ss_time;
        // Formato: [YYYY-MM-DD HH:MM:SS]
        ss_time << std::put_time(std::localtime(&in_time_t), "[%Y-%m-%d %H:%M:%S] ");
        // --- FIM DA ADIÇÃO DO TIMESTAMP ---

        ///< Esse lock_guard trava enquanto estiver nesse escopo
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            // Constrói a string final na memória RAM
            m_current_buffer.emplace_back(ss_time.str() + prefixo + msg);

            if( m_is_the_first ){ init_file();
                                  m_worker = std::thread(&Logger::worker_loop, this);
                                  m_is_the_first = false;
                                }
        }
        // Notifica a thread de escrita que há dados
        m_cv.notify_one();
    }

    /**
     * @brief Loop da thread de background, responsável por escrever no arquivo .log da melhor forma possível.
     * @details Função de alto nível
     */
    void
    worker_loop() {

        while(
            m_is_running || !m_current_buffer.empty()
        ){

            std::unique_lock<std::mutex> lock(m_mutex);

            ///< Espera até ter dados ou ser instruído a encerrar
            /*
            A thread fica bloqueada pelo sistema operacional, sem consumir CPU.
            Pesquise, isso é muito foda.
            */
            m_cv.wait(
                lock,
                [this](){ return !m_current_buffer.empty() || !m_is_running;}
            );

            if( m_current_buffer.empty() && !m_is_running ){ break; }

            // --- A MÁGICA DA PERFORMANCE (SWAP) ---
            // Trocamos o vetor cheio pelo vazio instantaneamente.
            // O Mutex é liberado logo depois disso.
            std::swap(m_current_buffer, m_write_buffer);
            lock.unlock();

            ///< Agora escrevemos no disco SEM bloquear quem quer adicionar logs
            if(m_file_stream.is_open()) {
                for(const auto& line : m_write_buffer){ m_file_stream << line << "\n"; }
                // Flush manual apenas após lote grande
                m_file_stream.flush();
            }

            // Limpa o buffer de escrita para ser reutilizado no próximo swap
            m_write_buffer.clear();
        }
    }

};