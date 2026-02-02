# BE-Tree vs A-Tree: Boolean Expression Matching Comparison

A comprehensive comparison of two boolean expression matching libraries with different algorithms and use cases.

## Table of Contents
- [Algorithm & Paper Basis](#algorithm--paper-basis)
- [Expression Capabilities](#expression-capabilities)
- [Data Types](#data-types)
- [Special Features](#special-features)
- [Use Cases](#use-cases)
- [Performance Characteristics](#performance-characteristics)
- [Code Examples](#code-examples)
- [Decision Guide](#decision-guide)

---

## Algorithm & Paper Basis

### BE-Tree
- Based on domain partitioning and tree-based indexing
- Focus on partition-based optimization with scoring functions
- Emphasis on memoization and short-circuiting
- Optimized for static expression sets with high-volume matching

### A-Tree
- Based on [A-Tree: A Dynamic Data Structure for Efficiently Indexing Arbitrary Boolean Expressions](https://dl.acm.org/doi/10.1145/3448016.3457266) (SIGMOD 2021)
- Focus on reusing intermediary nodes across expressions
- Dynamic restructuring as expressions are added/removed
- Optimized for evolving expression sets

---

## Expression Capabilities

### Common Features (Both Support)

| Feature | BE-Tree Syntax | A-Tree Syntax |
|---------|---------------|---------------|
| **Comparison** | `<`, `<=`, `>`, `>=` | `<`, `<=`, `>`, `>=` |
| **Equality** | `=`, `<>` | `=`, `!=` |
| **Boolean Logic** | `and`, `or`, `not` | `and`, `or`, `not` |
| **List Operations** | `one of`, `none of`, `all of` | `one_of`, `none_of`, `all_of` |
| **Set Operations** | `in`, `not in` | `in`, `not in` |
| **Null Checks** | `is null`, `is not null`, `is empty` | `is_null`, `is_not_null`, `is_empty`, `is_not_empty` |
| **Parentheses** | `(expression)` | `(expression)` |

### Expression Examples

**BE-Tree:**
```sql
age >= 18 and age <= 65
country = "USA" or country = "Canada"
premium = true and (score >= 80.0 or purchases > 10)
deal_ids one of [123, 456, 789]
price < 100.0 and category not in ["restricted", "banned"]
```

**A-Tree:**
```sql
age >= 18 and age <= 65
country = "USA" or country = "Canada"
private and exchange_id = 1
deal_ids one_of [123, 456, 789]
price < 100 and category not_in ["restricted", "banned"]
```

---

## Data Types

### BE-Tree Data Types

| Type | C Type | Range | Notes |
|------|--------|-------|-------|
| Boolean | `bool` | `false`, `true` | Direct boolean checks |
| Integer | `int64_t` | -2^63 to 2^63-1 | 64-bit signed |
| Float | `double` | ±DBL_MAX | IEEE 754 double precision |
| String | `char*` | - | Interned for fast comparison |
| Integer List | `int64_t[]` | - | Sorted for O(log n) lookups |
| String List | `char*[]` | - | Sorted for O(log n) lookups |
| Segments | `struct` | - | User segment membership with timestamps |
| Frequency Caps | `struct` | - | Ad frequency tracking with time windows |

### A-Tree Data Types

| Type | Rust Type | Range | Notes |
|------|-----------|-------|-------|
| Boolean | `bool` | `false`, `true` | Direct boolean checks |
| Integer | `i64` | -2^63 to 2^63-1 | 64-bit signed |
| String | `String` | - | UTF-8 strings |
| StringList | `Vec<String>` | - | Vector of strings |

**Key Difference:** BE-Tree supports floating-point arithmetic; A-Tree does not.

---

## Special Features

### BE-Tree Unique Features

#### 1. Frequency Capping (Ad Tech)

**Purpose:** Track and limit impression/click frequency per advertiser, campaign, flight, or product.

**Function:**
```c
bool within_frequency_caps(
    const struct betree_frequency_caps* caps,
    enum frequency_type_e type,
    uint32_t id,
    const struct string_value ns,
    uint32_t value,
    size_t length,
    int64_t now
)
```

**Frequency Types:**
- `FREQUENCY_TYPE_ADVERTISER` / `FREQUENCY_TYPE_ADVERTISERIP`
- `FREQUENCY_TYPE_CAMPAIGN` / `FREQUENCY_TYPE_CAMPAIGNIP`
- `FREQUENCY_TYPE_CAMPAIGN_GROUP` / `FREQUENCY_TYPE_CAMPAIGN_GROUPIP`
- `FREQUENCY_TYPE_FLIGHT` / `FREQUENCY_TYPE_FLIGHTIP`
- `FREQUENCY_TYPE_PRODUCT` / `FREQUENCY_TYPE_PRODUCTIP`

**Expression Example:**
```sql
-- Limit campaign to 3 impressions per day
within_frequency_caps(campaign_123, "impressions", 3, 86400, now)
```

#### 2. User Segments (Time-Based)

**Purpose:** Check if user entered or exited a segment within a time window.

**Functions:**
```c
bool segment_within(int64_t segment_id, int32_t after_seconds,
                    const struct betree_segments* segments, int64_t now)

bool segment_before(int64_t segment_id, int32_t before_seconds,
                    const struct betree_segments* segments, int64_t now)
```

**Expression Examples:**
```sql
-- User joined premium segment in last 7 days
segment_within(premium_users, 604800, now)

-- User left churned segment more than 30 days ago
segment_before(churned_users, 2592000, now)
```

#### 3. Geolocation

**Purpose:** Geographic proximity matching using Haversine formula.

**Function:**
```c
bool geo_within_radius(double lat1, double lon1, double lat2, double lon2, double distance)
```

**Expression Example:**
```sql
-- User within 50km of New York City
geo_within_radius(user_lat, user_lon, 40.7128, -74.0060, 50.0)
```

#### 4. String Operations

**Functions:**
```c
bool contains(const char* value, const char* pattern)
bool starts_with(const char* value, const char* pattern)
bool ends_with(const char* value, const char* pattern)
```

**Expression Examples:**
```sql
contains(url, "/product/")
starts_with(email, "admin")
ends_with(domain, ".com")
```

#### 5. Float Support

Critical for:
- Price comparisons: `price >= 19.99 and price < 99.99`
- Scoring: `relevance_score > 0.85`
- Percentages: `discount_rate <= 0.25`
- Statistical thresholds: `confidence >= 0.95`

### A-Tree Unique Features

#### 1. Dynamic Deletion

**Purpose:** Remove subscriptions without rebuilding the entire tree.

**Function:**
```c
AtreeResult atree_delete(ATreeHandle *tree, uint64_t subscription_id)
```

**Advantage:** Critical for dynamic rule engines where rules frequently change.

#### 2. Adaptive Restructuring

- Tree reorganizes itself as expressions are added/removed
- Maintains optimal structure automatically
- No manual rebalancing required

---

## Use Cases

### BE-Tree: Ad Tech / Marketing Automation

**Primary Use Cases:**
1. **Real-Time Bidding (RTB)**
   - Match ad campaigns to bid requests in <10ms
   - Evaluate millions of targeting rules per second
   - Frequency capping across campaigns
   - Geographic targeting

2. **Campaign Management**
   - User segmentation with time decay
   - A/B test variant assignment
   - Behavioral targeting
   - Lookalike audience matching

3. **Content Personalization**
   - Personalized content recommendations
   - Dynamic pricing rules
   - Promotion eligibility
   - Feature flag evaluation with numeric scores

**Example Expression:**
```sql
-- Complex ad targeting
age >= 18 and age <= 65
and country in ["USA", "Canada", "UK"]
and within_frequency_caps(campaign_123, "impressions", 3, 86400, now)
and segment_within(premium_users, 604800, now)
and geo_within_radius(user_lat, user_lon, store_lat, store_lon, 25.0)
and purchase_history_score > 75.0
and last_visit_days <= 7
```

### A-Tree: General-Purpose Rule Engines

**Primary Use Cases:**
1. **Content Filtering**
   - Message routing
   - Spam detection
   - Content moderation
   - Access control

2. **Event Routing**
   - Stream processing rules
   - Notification targeting
   - Workflow triggers
   - Alert routing

3. **Feature Flags**
   - Feature rollout rules
   - User cohort targeting
   - Canary deployment
   - Permission systems

**Example Expression:**
```rust
// Content filtering
private = true and exchange_id = 1 and status = "active"

// Event routing
priority > 5 and category in ["urgent", "critical"] and not archived

// Feature flags
user_tier = "premium" and beta_enabled and region = "US"
```

---

## Performance Characteristics

### BE-Tree Performance Features

**Optimization Techniques:**

1. **Memoization**
   - Caches sub-expression evaluation results
   - Reuses results when same sub-expression appears in multiple subscriptions
   - Tracked per-predicate with unique IDs

2. **Short-Circuiting**
   - Determines expression outcome before evaluation when possible
   - Uses bitmap to track which undefined attributes cause pass/fail
   - Skips evaluation entirely if outcome known

3. **Domain Partitioning**
   - Splits attribute value ranges into partitions
   - Routes events to relevant partitions only
   - Reduces expressions to evaluate by orders of magnitude

4. **String Interning**
   - Assigns numeric IDs to strings
   - String comparisons become integer comparisons
   - O(1) lookups instead of strcmp()

**Performance Profile:**
- ✅ Optimized for **millions of expressions**
- ✅ Sub-millisecond matching for thousands of expressions
- ✅ Highly efficient for **static expression sets**
- ⚠️ Large memory footprint (caching, partitions, memoization)
- ⚠️ No deletion support (requires rebuild)

**Typical Metrics:**
- Insert: O(log n) to O(n) depending on tree rebalancing
- Search: O(k) where k = expressions matched (often k << n)
- Memory: ~500 bytes per expression (varies with complexity)

### A-Tree Performance Features

**Optimization Techniques:**

1. **Node Reuse**
   - Shares common sub-expressions across subscriptions
   - Reduces memory usage
   - Single evaluation for shared nodes

2. **Dynamic Restructuring**
   - Tree adapts to expression distribution
   - Maintains balance automatically
   - Optimizes for current workload

3. **Efficient Updates**
   - Incremental insertion without full rebuild
   - Deletion without compaction overhead
   - Lock-free reads (Rust safety)

**Performance Profile:**
- ✅ Memory efficient through node reuse
- ✅ **Dynamic add/remove** with minimal overhead
- ✅ Well-suited for **evolving expression sets**
- ⚠️ Less specialized for specific domains
- ⚠️ Rust overhead for FFI calls from C/C++

**Typical Metrics:**
- Insert: O(m) where m = expression size
- Delete: O(m) where m = expression size
- Search: O(k × m) where k = matched expressions, m = avg expression size
- Memory: Reduced through node sharing

---

## Code Examples

### BE-Tree (C API)

```c
#include "betree.h"

// Create tree
struct betree* tree = betree_make();

// Define schema
betree_add_integer_variable(tree, "age", false, 0, 150);
betree_add_string_variable(tree, "country", false, 100);
betree_add_boolean_variable(tree, "premium", false);
betree_add_float_variable(tree, "score", false, 0.0, 100.0);

// Insert subscriptions
betree_insert(tree, 1, "age >= 18 and score > 80.0");
betree_insert(tree, 2, "country = \"USA\" and premium");

// Create event
const char* event = "{\"age\": 25, \"country\": \"USA\", \"premium\": true, \"score\": 85.5}";

// Search
struct report* report = make_report();
betree_search(tree, event, report);

// Check results
for (size_t i = 0; i < report->matched; i++) {
    printf("Matched subscription: %lu\n", report->subs[i]);
}

// Cleanup
free_report(report);
betree_free(tree);
```

### BE-Tree (C++ API)

```cpp
#include <betree_cpp.hpp>

// Create tree with fluent API
be::Tree tree;
tree.add_integer("age", false, 0, 150)
    .add_string("country", false, 100)
    .add_boolean("premium", false)
    .add_float("score", false, 0.0, 100.0);

// Insert subscriptions
tree.insert(1, "age >= 18 and score > 80.0");
tree.insert(2, "country = \"USA\" and premium");

// Search with automatic memory management
auto results = tree.search(R"({
    "age": 25,
    "country": "USA",
    "premium": true,
    "score": 85.5
})");

// Results in std::vector
for (auto sub_id : results.matched_subs) {
    std::cout << "Matched: " << sub_id << "\n";
}

// Automatic cleanup via RAII
```

### A-Tree (C API)

```c
#include "atree.h"

// Define attributes
AtreeAttributeDef defs[] = {
    { .name = "age", .attr_type = Integer },
    { .name = "country", .attr_type = String },
    { .name = "premium", .attr_type = Boolean }
};

// Create tree
ATreeHandle *tree = atree_new(defs, 3);

// Insert subscription
atree_insert(tree, 1, "age >= 18 and country = \"USA\"");

// Build event
void *builder = atree_event_builder_new(tree);
atree_event_builder_with_integer(builder, "age", 25);
atree_event_builder_with_string(builder, "country", "USA");
atree_event_builder_with_boolean(builder, "premium", true);

// Search (consumes builder)
AtreeSearchResult result = atree_search(tree, builder);

// Check results
for (size_t i = 0; i < result.count; i++) {
    printf("Matched: %lu\n", result.ids[i]);
}

// Cleanup
atree_search_result_free(result);
atree_free(tree);
```

### A-Tree (Rust API)

```rust
use a_tree::{ATree, AttributeDefinition, AttributeType, Event};

// Define schema
let attributes = vec![
    AttributeDefinition::new("age", AttributeType::Integer),
    AttributeDefinition::new("country", AttributeType::String),
    AttributeDefinition::new("premium", AttributeType::Boolean),
];

// Create tree
let mut tree = ATree::new(attributes);

// Insert subscription
tree.insert(1, "age >= 18 and country = \"USA\"").unwrap();

// Create event
let mut event = Event::new();
event.with_integer("age", 25);
event.with_string("country", "USA");
event.with_boolean("premium", true);

// Search
let matches = tree.search(&event);

for sub_id in matches {
    println!("Matched: {}", sub_id);
}
```

---

## Decision Guide

### Choose BE-Tree If:

✅ **You need domain-specific features:**
- Frequency capping for advertising
- Time-based user segmentation
- Geolocation targeting
- Float/decimal precision critical

✅ **Performance requirements:**
- Millions of expressions to evaluate
- Sub-millisecond latency critical
- Static or slowly-changing expression set
- High-volume event matching (>10k events/sec)

✅ **Use case matches:**
- Ad tech / RTB platforms
- Marketing automation
- Campaign management
- Personalization engines
- Recommendation systems

✅ **Trade-offs acceptable:**
- No deletion support (rebuild for updates)
- Higher memory footprint
- More complex setup

### Choose A-Tree If:

✅ **You need flexibility:**
- Dynamic rule insertion/deletion
- Frequently changing expression sets
- General-purpose boolean matching
- No domain-specific requirements

✅ **Simplicity preferred:**
- Cleaner, modern API
- Better error messages
- Rust safety guarantees (if using Rust)
- Lower learning curve

✅ **Use case matches:**
- Rule engines
- Event routing systems
- Feature flag platforms
- Content filtering
- Access control / permissions
- Workflow engines

✅ **Trade-offs acceptable:**
- No float support
- No special domain predicates
- Potentially lower raw throughput

---

## Feature Comparison Matrix

| Feature | BE-Tree | A-Tree |
|---------|---------|--------|
| **Algorithm** | Domain partitioning | Node reuse |
| **Language** | C → C++20 | Rust + C/C++ FFI |
| **License** | MIT | MIT / Apache 2.0 |
| **Year** | ~2016 | 2021 (paper) |
| | | |
| **Data Types** | | |
| Boolean | ✅ | ✅ |
| Integer | ✅ (64-bit) | ✅ (64-bit) |
| Float | ✅ (double) | ❌ |
| String | ✅ | ✅ |
| Integer List | ✅ | ❌ |
| String List | ✅ | ✅ |
| Custom Structs | ✅ | ❌ |
| | | |
| **Operations** | | |
| Comparison (<, <=, >, >=) | ✅ | ✅ |
| Equality (=, !=) | ✅ | ✅ |
| Boolean (and, or, not) | ✅ | ✅ |
| Set (in, not in) | ✅ | ✅ |
| List (one_of, all_of, none_of) | ✅ | ✅ |
| Null checks | ✅ | ✅ |
| String matching | ✅ (contains, etc.) | ❌ |
| | | |
| **Special Features** | | |
| Frequency Capping | ✅ | ❌ |
| User Segments | ✅ | ❌ |
| Geolocation | ✅ | ❌ |
| Deletion Support | ❌ | ✅ |
| | | |
| **Performance** | | |
| Memoization | ✅ | Partial |
| Short-circuiting | ✅ | No |
| Domain Partitioning | ✅ | No |
| Node Reuse | No | ✅ |
| Dynamic Updates | ❌ (rebuild) | ✅ |
| | | |
| **API Quality** | | |
| C API | ✅ | ✅ |
| C++ API | ✅ (modern) | ⚠️ (examples only) |
| Rust API | ❌ | ✅ |
| RAII Support | ✅ | ⚠️ (manual) |
| Error Handling | errno-style | Result types |
| Documentation | Good | Excellent |

---

## Performance Benchmarks

### BE-Tree Reported Performance

From real-world ad tech deployments:
- **Expression count:** 1M+ subscriptions
- **Match latency:** <1ms for 10K-100K candidate expressions
- **Throughput:** 10K+ events/second single-threaded
- **Memory:** ~500MB for 1M expressions (typical)

### A-Tree Reported Performance

From paper (SIGMOD 2021):
- **Insertion:** 10K-100K expressions/second
- **Deletion:** Similar to insertion
- **Search:** Depends on match count and expression complexity
- **Memory:** Significantly reduced through node sharing

---

## Migration Considerations

### From BE-Tree to A-Tree

**Pros:**
- ✅ Gain deletion support
- ✅ Simpler API and better errors
- ✅ Memory savings through node reuse

**Cons:**
- ❌ Lose float support (convert to scaled integers?)
- ❌ Lose frequency capping (implement externally)
- ❌ Lose geolocation (implement externally)
- ❌ Potential performance regression for high-volume matching

**Expression Changes:**
- `<>` → `!=` (not equal syntax)
- `one of` → `one_of` (spacing)
- Remove frequency/segment/geo predicates
- Convert floats to integers with scaling

### From A-Tree to BE-Tree

**Pros:**
- ✅ Gain float support
- ✅ Gain domain-specific features
- ✅ Better throughput for high-volume matching
- ✅ Modern C++ API with RAII

**Cons:**
- ❌ Lose deletion support (requires rebuild)
- ❌ Higher memory usage
- ❌ More complex setup

**Expression Changes:**
- `!=` → `<>` (not equal syntax)
- `one_of` → `one of` (spacing)
- No major syntax changes otherwise

---

## Conclusion

Both **BE-Tree** and **A-Tree** are excellent boolean expression matching libraries, each optimized for different scenarios:

- **BE-Tree** excels in ad tech and marketing automation with specialized features, high throughput, and comprehensive data type support including floats.

- **A-Tree** provides a cleaner, more modern implementation based on recent research, with dynamic update support and memory efficiency.

The choice depends on your specific requirements:
- Need frequency capping, segments, or geo? → **BE-Tree**
- Need to add/remove rules frequently? → **A-Tree**
- Need float support? → **BE-Tree**
- Want simpler API? → **A-Tree**
- Ad tech use case? → **BE-Tree**
- General rule engine? → **A-Tree**

---

## References

- **BE-Tree Repository:** https://github.com/mrayva/be-tree
- **A-Tree Repository:** https://github.com/mrayva/a-tree
- **A-Tree Paper:** [A-Tree: A Dynamic Data Structure for Efficiently Indexing Arbitrary Boolean Expressions](https://dl.acm.org/doi/10.1145/3448016.3457266) (SIGMOD 2021)

---

*This comparison was created based on analysis of both codebases as of 2026-02-02.*
