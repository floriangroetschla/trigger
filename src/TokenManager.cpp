#include "trigger/TokenManager.hpp"

namespace dunedaq::trigger {

TokenManager::TokenManager(std::unique_ptr<appfwk::DAQSource<dfmessages::TriggerDecisionToken>>& token_source,
                           int initial_tokens,
                           dataformats::run_number_t run_number)
  : m_n_initial_tokens(initial_tokens)
  , m_token_source(token_source)
  , m_run_number(run_number)

{
  m_running_flag.store(true);
  m_read_queue_thread = std::thread(&TokenManager::read_token_queue, this);
}

TokenManager::~TokenManager()
{
  m_running_flag.store(false);
  m_read_queue_thread.join();
}

int
TokenManager::get_n_tokens() const
{
  return m_n_tokens.load();
}

void
TokenManager::trigger_sent(dfmessages::trigger_number_t trigger_number)
{
  std::lock_guard<std::mutex> lk(m_open_trigger_decisions_mutex);
  m_open_trigger_decisions.insert(trigger_number);
  m_n_tokens--;
}

void
TokenManager::read_token_queue()
{
  auto open_trigger_report_time = std::chrono::steady_clock::now();
  while (m_running_flag.load()) {
    while (m_token_source->can_pop()) {
      dfmessages::TriggerDecisionToken tdt;
      m_token_source->pop(tdt);
      TLOG_DEBUG(1) << "Received token with run number " << tdt.run_number << ", current run number " << m_run_number;
      if (tdt.run_number == m_run_number) {
        m_n_tokens++;
        TLOG_DEBUG(1) << "There are now " << m_n_tokens.load() << " tokens available";

        if (tdt.trigger_number != dfmessages::TypeDefaults::s_invalid_trigger_number) {
          if (m_open_trigger_decisions.count(tdt.trigger_number)) {
            std::lock_guard<std::mutex> lk(m_open_trigger_decisions_mutex);
            m_open_trigger_decisions.erase(tdt.trigger_number);
            TLOG_DEBUG(1) << "Token indicates that trigger decision " << tdt.trigger_number
                          << " has been completed. There are now " << m_open_trigger_decisions.size()
                          << " triggers in flight";
          } else {
            // ERS warning: received token for trigger number I don't recognize
          }
        }
      }
    }
    if (!m_open_trigger_decisions.empty()) {

      auto now = std::chrono::steady_clock::now();
      if (std::chrono::duration_cast<std::chrono::milliseconds>(now - open_trigger_report_time) >
          std::chrono::milliseconds(3000)) {
        std::ostringstream o;
        o << "Open Trigger Decisions: [";
        { // Scope for lock_guard
          bool first = true;
          std::lock_guard<std::mutex> lk(m_open_trigger_decisions_mutex);
          for (auto& td : m_open_trigger_decisions) {
            if (!first)
              o << ", ";
            o << td;
            first = false;
          }
          o << "]";
        }
        TLOG_DEBUG(0) << o.str();
        open_trigger_report_time = now;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

} // dunedaq::trigger
