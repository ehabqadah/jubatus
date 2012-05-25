// Jubatus: Online machine learning framework for distributed environment
// Copyright (C) 2011,2012 Preferred Infrastructure and Nippon Telegraph and Telephone Corporation.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include <gtest/gtest.h>
#include <stdexcept>
#include <pficommon/lang/cast.h>
#include "graph_wo_index.hpp"

using namespace std;

namespace jubatus{
namespace graph {

namespace {

void mix_graphs(size_t count, vector<graph_wo_index>& gs) {
  string diff, mixed;
  while (count-- > 0) {
    mixed.clear();
    for (size_t i = 0; i < gs.size(); ++i) {
      gs[i].get_diff(diff);
      gs[0].mix(diff, mixed);
    }
    for (size_t i = 0; i < gs.size(); ++i) {
      gs[i].set_mixed_and_clear_diff(mixed);
    }
  }
}

}

TEST(graph_wo_index, none){
  graph_wo_index g;
  preset_query q;

  g.clear();
  ASSERT_EQ("graph_wo_index", g.type());

  EXPECT_THROW(g.update_node(7, property()), runtime_error);
  EXPECT_THROW(g.remove_global_node(7), runtime_error);
  EXPECT_THROW(g.update_edge(8, property()), runtime_error);
  EXPECT_THROW(g.remove_edge(8), runtime_error);
  EXPECT_THROW(g.centrality(9, EIGENSCORE, q), runtime_error);
  vector<node_id_t> ret;
  EXPECT_THROW(g.shortest_path(10, 11, 1000, ret, q), runtime_error);

  {
    map<string, string> status;
    g.get_status(status);
    ASSERT_EQ("0", status["local_node_num"]);
    ASSERT_EQ("0", status["local_edge_num"]);
    ASSERT_EQ("0", status["global_node_num"]);
  }

  g.create_node(777);
  g.create_global_node(777);
  {
    map<string, string> status;
    g.get_status(status);
    ASSERT_EQ("1", status["local_node_num"]);
    ASSERT_EQ("1", status["global_node_num"]);
  }
  g.create_global_node(888);
  g.create_node(888);
  g.create_global_node(999);
  {
    map<string, string> status;
    g.get_status(status);
    ASSERT_EQ("2", status["local_node_num"]);
    ASSERT_EQ("3", status["global_node_num"]);
  }

  g.create_edge(10000, 777, 999);
  g.create_edge(10001, 777, 888);
  g.create_edge(10002, 777, 999);
  EXPECT_THROW(g.create_edge(10002, 4, 5), runtime_error);
  EXPECT_THROW(g.create_edge(10002, 999, 5), runtime_error);
  {
    map<string, string> status;
    g.get_status(status);
    ASSERT_EQ("2", status["local_node_num"]);
    ASSERT_EQ("3", status["local_edge_num"]);
    ASSERT_EQ("3", status["global_node_num"]);
  }

  {
    edge_info ei;
    g.get_edge(10000, ei);
    ASSERT_EQ(777u, ei.src);
    ASSERT_EQ(999u, ei.tgt);
  }

  {
    node_info ni;
    g.get_node(777, ni);
    ASSERT_EQ(0u, ni.in_edges.size());
    ASSERT_EQ(3u, ni.out_edges.size());
  }
  {
    node_info ni;
    g.get_node(888, ni);
    ASSERT_EQ(1u, ni.in_edges.size());
    ASSERT_EQ(0u, ni.out_edges.size());
    ASSERT_EQ(10001u, ni.in_edges[0]);
  }

  EXPECT_THROW(g.remove_node(5), runtime_error);
  EXPECT_THROW(g.remove_node(999), runtime_error);
  EXPECT_THROW(g.remove_edge(5), runtime_error);
  EXPECT_THROW(g.remove_node(777), runtime_error); // edge is associated
  EXPECT_THROW(g.remove_node(888), runtime_error); // edge is associated

  g.remove_edge(10000);
  g.remove_edge(10001);
  g.remove_node(888);
  g.remove_global_node(888);
  {
    map<string, string> status;
    g.get_status(status);
    ASSERT_EQ("1", status["local_node_num"]);
    ASSERT_EQ("1", status["local_edge_num"]);
    ASSERT_EQ("2", status["global_node_num"]);
  }

  g.update_index();
}

TEST(graph, random){
  graph_wo_index g;

  const uint64_t node_num = 1000;
  uint64_t local_node_num = 0;
  set<uint64_t> local_ids;
  for (uint64_t i = 0; i < node_num; ++i){
    g.create_global_node(i);
    if ((rand() % 10) == 0){
      g.create_node(i);
      local_ids.insert(i);
      ++local_node_num;
    }
  }
  
  const uint64_t edge_num = 10000;
  uint64_t edge_added_num = 0;
  for (uint64_t i = 0; i < edge_num; ++i){
    uint64_t src = rand() % node_num;
    uint64_t tgt = rand() % node_num;
    if (src == tgt) continue;
    if (local_ids.count(src) == 0 && local_ids.count(tgt) == 0) continue;
    g.create_edge(i, src, tgt);
    ++edge_added_num;
  }
  
  map<string, string> status;
  g.get_status(status);
  ASSERT_EQ(pfi::lang::lexical_cast<string>(local_node_num), status["local_node_num"]);
  ASSERT_EQ(pfi::lang::lexical_cast<string>(node_num), status["global_node_num"]);
  ASSERT_EQ(pfi::lang::lexical_cast<string>(edge_added_num), status["local_edge_num"]);

  
}

TEST(graph, mix){
  graph_wo_index g;
  string diff;
  g.get_diff(diff);
  string mixed;
  g.mix(diff, mixed);
  g.set_mixed_and_clear_diff(mixed);
}

TEST(graph, shortest_path_line_graph) {
  graph_wo_index g;
  g.add_shortest_path_query(preset_query());

  g.create_global_node(1);
  g.create_node(1);
  g.create_global_node(2);
  g.create_node(2);
  g.create_global_node(3);
  g.create_node(3);

  g.create_edge(13, 1, 3);
  g.create_edge(21, 2, 1);

  string diff, mixed;
  g.get_diff(diff);
  g.mix(diff, mixed);
  g.set_mixed_and_clear_diff(mixed);

  vector<node_id_t> path;
  g.shortest_path(2, 3, 3, path, preset_query());

  ASSERT_EQ(3u, path.size());
  EXPECT_EQ(2u, path[0]);
  EXPECT_EQ(1u, path[1]);
  EXPECT_EQ(3u, path[2]);
}

TEST(graph, shortest_path_line_graph_with_mix) {
  // V = { 1, 2, 3 }, E = { (1, 2), (2, 3) }.
  // V1 = { 1 }, V2 = { 2, 3 }.
  // query to V2: Find shortest path from 1 to 3.

  vector<graph_wo_index> g(2);
  g[0].add_shortest_path_query(preset_query());
  g[1].add_shortest_path_query(preset_query());

  for (node_id_t i = 1; i <= 3u; ++i) {
    g[0].create_global_node(i);
    g[1].create_global_node(i);
  }

  g[0].create_node(1);
  g[1].create_node(2);
  g[1].create_node(3);

  g[0].create_edge(12, 1, 2);
  g[1].create_edge(12, 1, 2);
  g[1].create_edge(23, 2, 3);

  mix_graphs(2, g);

  vector<node_id_t> path;
  g[1].shortest_path(1, 3, 3, path, preset_query());

  ASSERT_EQ(3u, path.size());
  EXPECT_EQ(1u, path[0]);
  EXPECT_EQ(2u, path[1]);
  EXPECT_EQ(3u, path[2]);
}

TEST(graph, shortest_path_line_graph_with_two_bridges) {
  // V = { 1, 2, 3, 4 }, E = { (1, 2), (2, 3), (3, 4) }.
  // V1 = { 1, 4 }, V2 = { 2, 3 }.
  // query: Find the shortest path from 1 to 4.

  vector<graph_wo_index> g(2);
  g[0].add_shortest_path_query(preset_query());
  g[1].add_shortest_path_query(preset_query());

  for (node_id_t i = 1; i <= 4u; ++i) {
    g[0].create_global_node(i);
    g[1].create_global_node(i);
  }

  g[0].create_node(1);
  g[1].create_node(2);
  g[1].create_node(3);
  g[0].create_node(4);

  g[0].create_edge(12, 1, 2);
  g[1].create_edge(12, 1, 2);
  g[1].create_edge(23, 2, 3);
  g[0].create_edge(34, 3, 4);
  g[1].create_edge(34, 3, 4);

  mix_graphs(2, g);

  vector<node_id_t> expect;
  expect.push_back(1);
  expect.push_back(2);
  expect.push_back(3);
  expect.push_back(4);

  for (size_t i = 0; i < g.size(); ++i) {
    vector<node_id_t> actual;
    g[i].shortest_path(1, 4, 4, actual, preset_query());
    EXPECT_EQ(expect, actual);
  }
}

TEST(graph, shortest_path_line_graph_with_three_bridges) {
  // V = { 1, ..., 5 }, E = { (1, 2), (2, 3), (3, 4), (4, 5) }.
  // V1 = { 1, 3, 5 }, V2 = { 2, 4 }.
  // query: Find the shortest path from 1 to 5.

  vector<graph_wo_index> g(2);
  g[0].add_shortest_path_query(preset_query());
  g[1].add_shortest_path_query(preset_query());

  for (node_id_t i = 1; i <= 5u; ++i) {
    g[0].create_global_node(i);
    g[1].create_global_node(i);
  }

  g[0].create_node(1);
  g[1].create_node(2);
  g[0].create_node(3);
  g[1].create_node(4);
  g[0].create_node(5);

  for (size_t i = 0; i < g.size(); ++i) {
    g[i].create_edge(12, 1, 2);
    g[i].create_edge(23, 2, 3);
    g[i].create_edge(34, 3, 4);
    g[i].create_edge(45, 4, 5);
  }

  mix_graphs(3, g);

  vector<node_id_t> expect;
  expect.push_back(1);
  expect.push_back(2);
  expect.push_back(3);
  expect.push_back(4);
  expect.push_back(5);

  for (size_t i = 0; i < g.size(); ++i) {
    vector<node_id_t> actual;
    g[i].shortest_path(1, 5, 5, actual, preset_query());
    EXPECT_EQ(expect, actual);
  }
}

TEST(graph, shortest_path_line_graph_with_node_query_failure) {
  // V = { 1, 2, 3 }, E = { (1, 2), (2, 3) }.
  // V1 = { 1 }, V2 = { 2, 3 }.
  // Nodes 1 and 3 have a property { "aaa": "bbb" }.
  // Node 2 has a property { "aaa": "ccc" }.
  // query: Find the shortest path from 1 to 3 with node property { "aaa": "bbb" }.

  preset_query query;
  query.node_query.push_back(make_pair("aaa", "bbb"));

  map<string, string> prop;
  prop["aaa"] = "bbb";

  map<string, string> wrong_prop;
  wrong_prop["aaa"] = "ccc";

  vector<graph_wo_index> g(2);
  g[0].add_shortest_path_query(query);
  g[1].add_shortest_path_query(query);

  for (node_id_t i = 1; i <= 3u; ++i) {
    g[0].create_global_node(i);
    g[1].create_global_node(i);
  }

  g[0].create_node(1);
  g[0].update_node(1, prop);
  g[1].create_node(2);
  g[1].update_node(2, wrong_prop);
  g[1].create_node(3);
  g[1].update_node(3, wrong_prop);

  g[0].create_edge(12, 1, 2);
  g[1].create_edge(12, 1, 2);
  g[1].create_edge(23, 2, 3);

  mix_graphs(2, g);

  for (size_t i = 0; i < g.size(); ++i) {
    vector<node_id_t> path;
    g[i].shortest_path(1, 3, 3, path, query);
    EXPECT_EQ(0u, path.size());
  }
}

TEST(graph, shortest_path_line_graph_two_bridges_failure) {
  // V = { 1, 2, 3 }, E = { (1, 2), (2, 3) }.
  // V1 = { 1, 3 }, V2 = { 2 }.
  // Nodes 1 and 3 have a property { "aaa": "bbb" }.
  // Node 2 has no property.
  // query: Find the shortest path from 1 to 3 with node property { "aaa": "bbb" }.

  preset_query query;
  query.node_query.push_back(make_pair("aaa", "bbb"));

  map<string, string> prop;
  prop["aaa"] = "bbb";

  vector<graph_wo_index> g(2);
  g[0].add_shortest_path_query(query);
  g[1].add_shortest_path_query(query);

  for (node_id_t i = 1; i <= 3u; ++i) {
    g[0].create_global_node(i);
    g[1].create_global_node(i);
  }

  g[0].create_node(1);
  g[0].update_node(1, prop);
  g[1].create_node(2);
  // Node 2 has no property
  g[0].create_node(3);
  g[0].update_node(3, prop);

  for (size_t i = 0; i < g.size(); ++i) {
    g[i].create_edge(12, 1, 2);
    g[i].create_edge(23, 2, 3);
  }

  mix_graphs(2, g);

  for (size_t i = 0; i < g.size(); ++i) {
    vector<node_id_t> actual;
    g[i].shortest_path(1, 3, 3, actual, query);
    EXPECT_EQ(0u, actual.size());
  }
}

TEST(graph, shortest_path_line_graph_with_edge_query_failure) {
  // V = { 1, 2, 3, 4 }, E = { (1, 2), (2, 3), (3, 4) }.
  // V1 = { 1, 2 }, V2 = { 3, 4 }.
  // Edges except (2, 3) have a property { "aaa": "bbb" }.
  // Edge (2, 3) has a property { "aaa": "ccc" }.
  // query: Find the shortest path from 1 to 4 with edge property { "aaa": "bbb" }.

  preset_query query;
  query.edge_query.push_back(make_pair("aaa", "bbb"));

  map<string, string> prop;
  prop["aaa"] = "bbb";

  map<string, string> wrong_prop;
  wrong_prop["aaa"] = "ccc";

  vector<graph_wo_index> g(2);
  g[0].add_shortest_path_query(query);
  g[1].add_shortest_path_query(query);

  for (node_id_t i = 1; i <= 4u; ++i) {
    g[0].create_global_node(i);
    g[1].create_global_node(i);
  }

  g[0].create_node(1);
  g[0].create_node(2);
  g[1].create_node(3);
  g[1].create_node(4);

  g[0].create_edge(12, 1, 2);
  g[0].update_edge(12, prop);
  g[0].create_edge(23, 2, 3);
  g[0].update_edge(23, wrong_prop);
  g[1].create_edge(23, 2, 3);
  g[1].update_edge(23, wrong_prop);
  g[1].create_edge(34, 3, 4);
  g[1].update_edge(34, prop);

  mix_graphs(2, g);

  for (size_t i = 0; i < g.size(); ++i) {
    vector<node_id_t> path;
    g[i].shortest_path(1, 4, 4, path, query);
    EXPECT_EQ(0u, path.size());
  }
}

TEST(graph, shortest_path_longcut_caused_by_node_query) {
  // V = { 1, ..., 5 }, V1 = { 1, 2 }, V2 = { 3, 4, 5 }.
  // E = { (1, 3), (3, 5), (5, 2), (1, 4), (4, 2) }.
  // Nodes except 4 have a property { "aaa": "bbb" }.
  // Node 4 does not have any property.
  // query: Find the shortest path from 1 to 2 with property { "aaa": "bbb" }.

  preset_query query;
  query.node_query.push_back(make_pair("aaa", "bbb"));

  map<string, string> prop;
  prop["aaa"] = "bbb";

  vector<graph_wo_index> g(2);
  g[0].add_shortest_path_query(query);
  g[1].add_shortest_path_query(query);

  for (node_id_t i = 1; i <= 5u; ++i) {
    g[0].create_global_node(i);
    g[1].create_global_node(i);
  }

  g[0].create_node(1);
  g[0].update_node(1, prop);
  g[0].create_node(2);
  g[0].update_node(2, prop);
  g[1].create_node(3);
  g[1].update_node(3, prop);
  g[1].create_node(4);
  // Node 4 has no property
  g[1].create_node(5);
  g[1].update_node(5, prop);

  g[0].create_edge(13, 1, 3);
  g[1].create_edge(13, 1, 3);
  g[1].create_edge(35, 3, 5);
  g[0].create_edge(52, 5, 2);
  g[1].create_edge(52, 5, 2);
  g[0].create_edge(14, 1, 4);
  g[1].create_edge(14, 1, 4);
  g[0].create_edge(42, 4, 2);
  g[1].create_edge(42, 4, 2);

  mix_graphs(2, g);

  vector<node_id_t> expect;
  expect.push_back(1);
  expect.push_back(3);
  expect.push_back(5);
  expect.push_back(2);

  for (size_t i = 0; i < g.size(); ++i) {
    vector<node_id_t> actual;
    g[i].shortest_path(1, 2, 4, actual, query);
    EXPECT_EQ(expect, actual);
  }
}

TEST(graph, shortest_path_node_query_failure_at_non_landmark_point) {
  // V = { 1, ..., 6 }, V1 = { 1 }, V2 = { 2 }, V3 = { 3, 4, 5 }, V4 = { 6 }.
  // E = { (1, 3), (3, 4), (4, 5), (5, 2), (1, 6), (6, 2) }.
  // Nodes except 6 have a property { "aaa": "bbb" }.
  // Node 6 does not have any property.
  // query: Find the shortest path from 1 to 2 with node property { "aaa": "bbb" }.

  preset_query query;
  query.node_query.push_back(make_pair("aaa", "bbb"));

  map<string, string> prop;
  prop["aaa"] = "bbb";

  vector<graph_wo_index> g(4);
  for (size_t i = 0; i < g.size(); ++i) {
    g[i].add_shortest_path_query(query);
  }

  for (size_t i = 0; i < g.size(); ++i) {
    for (node_id_t j = 1; j <= 6u; ++j) {
      g[i].create_global_node(j);
    }
  }

  g[0].create_node(1);
  g[0].update_node(1, prop);
  g[1].create_node(2);
  g[1].update_node(2, prop);
  g[2].create_node(3);
  g[2].update_node(3, prop);
  g[2].create_node(4);
  g[2].update_node(4, prop);
  g[2].create_node(5);
  g[2].update_node(5, prop);

  g[0].create_edge(13, 1, 3);
  g[2].create_edge(13, 1, 3);
  g[2].create_edge(34, 3, 4);
  g[2].create_edge(45, 4, 5);
  g[1].create_edge(52, 5, 2);
  g[2].create_edge(52, 5, 2);

  mix_graphs(2, g);  // Nodes 1, ..., 5 become landmarks

  g[3].create_node(6);
  // Node 6 has no property

  g[0].create_edge(16, 1, 6);
  g[3].create_edge(16, 1, 6);
  g[1].create_edge(62, 6, 2);
  g[3].create_edge(62, 6, 2);

  mix_graphs(2, g);  // Node 6 is not a landmark

  vector<node_id_t> expect;
  expect.push_back(1);
  expect.push_back(3);
  expect.push_back(4);
  expect.push_back(5);
  expect.push_back(2);

  for (size_t i = 0; i < g.size(); ++i) {
    vector<node_id_t> actual;
    g[i].shortest_path(1, 2, 5, actual, query);
    EXPECT_EQ(expect, actual);
  }
}

}
}
