#include <chrono>
#include "app/GeneradorEventos.h"
#include "app/LectorConfig.h"
#include "lib/web/GHttpClient.h"
#include "lib/utils/Logger.h"

static constexpr const char* LOG_COMPONENT = "GeneradorEventos";

void GeneradorEventos::configure()
{
    const auto cfg = LectorConfig::getInstance().getParams();

    const long maxIdenCfg   = cfg->getStringLong("maxQueueIden", 120);
    const long maxNoIdenCfg = cfg->getStringLong("maxQueueNoIden", 80);

    maxQueueIden   = (maxIdenCfg   < 0) ? 0 : (size_t)maxIdenCfg;
    maxQueueNoIden = (maxNoIdenCfg < 0) ? 0 : (size_t)maxNoIdenCfg;

    LOG_INFO(LOG_COMPONENT,
        "CONFIG maxQueueIden=" << maxQueueIden
        << " maxQueueNoIden=" << maxQueueNoIden
        << " endpointUnificado=" << (usarEndpointUnificado ? "S" : "N"));
}

void GeneradorEventos::agregarTramaIden(string trama)
{
    mtxBloquea();

    const size_t before = (size_t)lstTramasPendIden.size();
    if (!canEnqueueIden())
    {
        const size_t maxQ = maxQueueIden;
        mtxLibera();
        LOG_INFO(LOG_COMPONENT, "DROP IDENT (cola llena). size=" << before << " max=" << maxQ);
        return;
    }

    lstTramasPendIden.add(std::move(trama));
    const size_t after = (size_t)lstTramasPendIden.size();
    const size_t maxQ = maxQueueIden;
    mtxLibera();

    LOG_INFO(LOG_COMPONENT, "ENQUEUE IDENT size " << before << "->" << after << " max=" << maxQ);
}

void GeneradorEventos::agregarTramaNoIden(string trama)
{
    mtxBloquea();

    const size_t before = (size_t)lstTramasPendNoIden.size();
    if (!canEnqueueNoIden())
    {
        const size_t maxQ = maxQueueNoIden;
        mtxLibera();
        LOG_INFO(LOG_COMPONENT, "DROP NO_IDENT (cola llena). size=" << before << " max=" << maxQ);
        return;
    }

    lstTramasPendNoIden.add(std::move(trama));
    const size_t after = (size_t)lstTramasPendNoIden.size();
    const size_t maxQ = maxQueueNoIden;
    mtxLibera();

    LOG_INFO(LOG_COMPONENT, "ENQUEUE NO_IDENT size " << before << "->" << after << " max=" << maxQ);
}

bool GeneradorEventos::hayEventosPend()
{
    bool rpta;

    mtxBloquea();
    rpta =
        (lstTramasPendIden.size() > 0) ||
        (lstTramasPendNoIden.size() > 0) ||
        (lstDeadIden.size() > 0) ||
        (lstDeadNoIden.size() > 0);
    mtxLibera();

    return rpta;
}

string GeneradorEventos::resolveUrl(TipoEvento tipo) const
{
    if (usarEndpointUnificado)
        return urlServidorUnificado;

    return (tipo == TipoEvento::Ident) ? urlServidorIden : urlServidorNoIden;
}

bool GeneradorEventos::tryPopNext(EventoPop& out)
{
    mtxBloquea();

    const bool hasMainIden   = (lstTramasPendIden.size() > 0);
    const bool hasMainNoIden = (lstTramasPendNoIden.size() > 0);
    const bool mainVacia = (!hasMainIden && !hasMainNoIden);

    const bool hasDeadIden   = (lstDeadIden.size() > 0);
    const bool hasDeadNoIden = (lstDeadNoIden.size() > 0);

    if (!mainVacia)
    {
        out.fuente = FuenteCola::Main;

        if (hasMainIden)
        {
            out.tipo = TipoEvento::Ident;
            out.trama = lstTramasPendIden.get(0);
            lstTramasPendIden.remove(0);
        }
        else
        {
            out.tipo = TipoEvento::NoIdent;
            out.trama = lstTramasPendNoIden.get(0);
            lstTramasPendNoIden.remove(0);
        }
    }
    else
    {
        if (!hasDeadIden && !hasDeadNoIden)
        {
            mtxLibera();
            return false;
        }

        out.fuente = FuenteCola::Dead;

        if (hasDeadIden)
        {
            out.tipo = TipoEvento::Ident;
            out.trama = lstDeadIden.get(0);
            lstDeadIden.remove(0);
        }
        else
        {
            out.tipo = TipoEvento::NoIdent;
            out.trama = lstDeadNoIden.get(0);
            lstDeadNoIden.remove(0);
        }
    }

    out.url = resolveUrl(out.tipo);

    out.pendI = (size_t)lstTramasPendIden.size();
    out.pendN = (size_t)lstTramasPendNoIden.size();
    out.deadI = (size_t)lstDeadIden.size();
    out.deadN = (size_t)lstDeadNoIden.size();

    mtxLibera();
    return true;
}

GeneradorEventos::SendResult GeneradorEventos::sendTrama(GHttpClient& httpClient, const EventoPop& ev)
{
    auto ini = std::chrono::high_resolution_clock::now();

    SendResult sr;
    sr.code = httpClient.doHttp(
        ev.url,
        "POST",
        (char*)ev.trama.c_str(),
        (int)ev.trama.length()
    );

    auto fin = std::chrono::high_resolution_clock::now();
    sr.durMs = (long)std::chrono::duration_cast<std::chrono::milliseconds>(fin - ini).count();
    sr.httpStatus = httpClient.httpError;

    return sr;
}

void GeneradorEventos::logDequeue(const EventoPop& ev) const
{
    LOG_INFO(LOG_COMPONENT,
        "DEQUEUE pop tipo=" << (ev.tipo == TipoEvento::Ident ? "IDENT" : "NO_IDENT")
        << " src=" << (ev.fuente == FuenteCola::Dead ? "DEAD" : "MAIN")
        << " pendI=" << ev.pendI << " pendN=" << ev.pendN
        << " deadI=" << ev.deadI << " deadN=" << ev.deadN);
}

void GeneradorEventos::onSendOk(const EventoPop& ev, const SendResult& sr) const
{
    LOG_INFO(LOG_COMPONENT,
        "SEND OK tipo=" << (ev.tipo == TipoEvento::Ident ? "IDENT" : "NO_IDENT")
        << " src=" << (ev.fuente == FuenteCola::Dead ? "DEAD" : "MAIN")
        << " httpStatus=" << sr.httpStatus
        << " durHttp=" << sr.durMs << "ms");

    if (generarLogEventos)
        LOG_DEBUG(LOG_COMPONENT, "Trama enviada (resumen): " << ev.trama);
}

bool GeneradorEventos::tryEnqueueDead(string&& trama, TipoEvento tipo)
{
    bool ok = false;

    mtxBloquea();

    if (tipo == TipoEvento::Ident)
    {
        if ((size_t)lstDeadIden.size() < maxQueueIden)
        {
            lstDeadIden.add(std::move(trama));
            ok = true;
        }
    }
    else
    {
        if ((size_t)lstDeadNoIden.size() < maxQueueNoIden)
        {
            lstDeadNoIden.add(std::move(trama));
            ok = true;
        }
    }

    mtxLibera();
    return ok;
}

void GeneradorEventos::onSendFail(EventoPop& ev, const SendResult& sr)
{
    if (ev.fuente == FuenteCola::Dead)
    {
        LOG_ERROR(LOG_COMPONENT,
            "SEND FAIL tipo=" << (ev.tipo == TipoEvento::Ident ? "IDENT" : "NO_IDENT")
            << " src=DEAD"
            << " code=" << sr.code
            << " httpStatus=" << sr.httpStatus
            << " durHttp=" << sr.durMs << "ms"
            << " action=DROP_FINAL"
            << " url=" << ev.url);
        return;
    }

    const bool movedToDead = tryEnqueueDead(std::move(ev.trama), ev.tipo);

    LOG_ERROR(LOG_COMPONENT,
        "SEND FAIL tipo=" << (ev.tipo == TipoEvento::Ident ? "IDENT" : "NO_IDENT")
        << " src=MAIN"
        << " code=" << sr.code
        << " httpStatus=" << sr.httpStatus
        << " durHttp=" << sr.durMs << "ms"
        << " action=" << (movedToDead ? "TO_DEAD" : "DROP_DEAD_FULL")
        << " url=" << ev.url);
}

void GeneradorEventos::runThread()
{
    LOG_INFO(LOG_COMPONENT, "Thread Generador Eventos iniciado");

    GHttpClient httpClient;
    httpClient.setHeader("Content-Type", "application/json");
    httpClient.setHeader("Connection", "close");

    while (!isFinalizado())
    {
        if (!enabled.load())
        {
            sleepMS(200);
            continue;
        }

        EventoPop ev;
        if (!tryPopNext(ev))
        {
            sleepMS(5);
            continue;
        }

        logDequeue(ev);

        const SendResult sr = sendTrama(httpClient, ev);

        if (sr.code == 0)
        {
            onSendOk(ev, sr);
        }
        else
        {
            onSendFail(ev, sr);
            sleepMS(25);
        }
    }

    LOG_INFO(LOG_COMPONENT, "Thread Generador Eventos finalizado");
}
