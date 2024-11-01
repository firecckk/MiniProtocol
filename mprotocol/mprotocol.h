#ifndef MPROTOCOL_H
#define MPROTOCOL_H

/**
 * @file mprotocol.h
 * @brief This file contains definitions and structures for the communication protocol.
 */

#include "packet.h"

#include "user_config.h"
#ifdef IS_HOST
#include "user_fields.h"
#include "host.h"
#else
#include "client.h"
#endif

#endif // MPROTOCOL_H