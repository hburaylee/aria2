/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2006 Tatsuhiro Tsujikawa
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
/* copyright --> */
#include "DHTFindNodeReplyMessage.h"

#include <cstring>

#include "DHTNode.h"
#include "DHTBucket.h"
#include "DHTRoutingTable.h"
#include "DHTMessageFactory.h"
#include "DHTMessageDispatcher.h"
#include "DHTMessageCallback.h"
#include "PeerMessageUtil.h"
#include "Util.h"
#include "bencode.h"

namespace aria2 {

const std::string DHTFindNodeReplyMessage::FIND_NODE("find_node");

const std::string DHTFindNodeReplyMessage::NODES("nodes");

DHTFindNodeReplyMessage::DHTFindNodeReplyMessage(const SharedHandle<DHTNode>& localNode,
						 const SharedHandle<DHTNode>& remoteNode,
						 const std::string& transactionID):
  DHTResponseMessage(localNode, remoteNode, transactionID) {}

DHTFindNodeReplyMessage::~DHTFindNodeReplyMessage() {}

void DHTFindNodeReplyMessage::doReceivedAction()
{
  for(std::deque<SharedHandle<DHTNode> >::iterator i = _closestKNodes.begin(); i != _closestKNodes.end(); ++i) {
    if(memcmp((*i)->getID(), _localNode->getID(), DHT_ID_LENGTH) != 0) {
      _routingTable->addNode(*i);
    }
  }
}

bencode::BDE DHTFindNodeReplyMessage::getResponse()
{
  bencode::BDE aDict = bencode::BDE::dict();
  aDict[DHTMessage::ID] = bencode::BDE(_localNode->getID(), DHT_ID_LENGTH);
  size_t offset = 0;
  unsigned char buffer[DHTBucket::K*26];
  // TODO if _closestKNodes.size() > DHTBucket::K ??
  for(std::deque<SharedHandle<DHTNode> >::const_iterator i =
	_closestKNodes.begin();
      i != _closestKNodes.end() && offset < DHTBucket::K*26; ++i) {
    SharedHandle<DHTNode> node = *i;
    memcpy(buffer+offset, node->getID(), DHT_ID_LENGTH);
    if(PeerMessageUtil::createcompact(buffer+20+offset, node->getIPAddress(),
				      node->getPort())) {
      offset += 26;
    }
  }
  aDict[NODES] = bencode::BDE(buffer, offset);
  return aDict;
}

std::string DHTFindNodeReplyMessage::getMessageType() const
{
  return FIND_NODE;
}

void DHTFindNodeReplyMessage::validate() const {}

const std::deque<SharedHandle<DHTNode> >& DHTFindNodeReplyMessage::getClosestKNodes() const
{
  return _closestKNodes;
}

void DHTFindNodeReplyMessage::setClosestKNodes(const std::deque<SharedHandle<DHTNode> >& closestKNodes)
{
  _closestKNodes = closestKNodes;
}

std::string DHTFindNodeReplyMessage::toStringOptional() const
{
  return "nodes="+Util::uitos(_closestKNodes.size());
}

} // namespace aria2
