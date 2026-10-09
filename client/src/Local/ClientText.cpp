#include "stdafx.h"
#include "Local/ClientText.h"

CClientText gClientText;
namespace {
const char* const English[] = {
    "%.10s || %s: %u || PING: %s || FPS: %.0f",
    "Success rate: pending",
    "Cost: pending",
    "Events",
    "Waiting for event information",
    "No scheduled events",
    "Disabled",
    "Open now",
    "In progress",
    "%lu days",
    "Teleport Window",
    "Map",
    "VIP",
    "Waiting for move information",
    "No destinations",
    "< Previous",
    "Next >"
};
static_assert(sizeof(English) / sizeof(English[0]) == (size_t)ClientTextId::Count,
              "Faltan textos del cliente");
}
const char* CClientText::Get(ClientTextId id) const
{
    const size_t index = (size_t)id;
    return index < (size_t)ClientTextId::Count ? English[index] : "";
}
