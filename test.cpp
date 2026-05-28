#include "bindings/wit.h"
#include "bindings/http_trigger_cpp.h"
#include "expected/expected"
#include <span>

namespace conv {
    std::span<const uint8_t> spanify(const char* chs, size_t sz) {
        return std::span(reinterpret_cast<uint8_t*>(const_cast<char*>(chs)), sz);
    }
    std::span<const uint8_t> spanify(const char* chs) {
        return spanify(chs, strlen(chs));
    }
}

namespace exports::wasi::http0_2_0::incoming_handler {
    void Handle(::wasi::http0_2_0::types::IncomingRequest&& request, ::wasi::http0_2_0::types::ResponseOutparam&& response_out) {
        // something goes amiss with multiple printfs: using explicit stdout works around it
        // auto out = ::wasi::cli0_2_0::stdout_::GetStdout();
        // out.BlockingWriteAndFlush(conv::spanify("got here\n"));

        ::wasi::http0_2_0::types::Fields headers;
        ::wasi::http0_2_0::types::OutgoingResponse resp(std::move(headers));
        auto ogbod = resp.Body().value();
        ::wasi::http0_2_0::types::ResponseOutparam::Set(std::move(response_out), std::move(resp));

        auto pq = request.PathWithQuery();

        if (pq.has_value()) {
            const auto pq_text = pq.value();
            const auto pq_text_len = pq_text.size();
            const auto pq_text_chars = pq_text.data();
            auto pq_text_span = conv::spanify(pq_text_chars, pq_text_len);

            auto out_stm = ogbod.Write().value();
            out_stm.BlockingWriteAndFlush(pq_text_span);
            out_stm.BlockingWriteAndFlush(conv::spanify("\n"));
        } else {
            auto out_stm = ogbod.Write().value();
            out_stm.BlockingWriteAndFlush(conv::spanify("no path-and-query\n"));
        }

        ::wasi::http0_2_0::types::OutgoingBody::Finish(std::move(ogbod), std::nullopt);
    }
}
