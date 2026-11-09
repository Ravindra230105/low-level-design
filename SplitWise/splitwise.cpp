
#include <bits/stdc++.h>
using namespace std;

using Money = long long; // Amount in paise

enum class SplitType {
    EQUAL,
    EXACT,
    PERCENTAGE,
    SHARES
};

enum class MemberStatus {
    ACTIVE,
    REMOVED
};

enum class ExpenseStatus {
    ACTIVE,
    DELETED
};

enum class LedgerType {
    EXPENSE,
    REVERSAL,
    SETTLEMENT
};

// -------------------- User --------------------

class User {
private:
    int userId;
    string name;
    string email;

public:
    User(int userId, string name, string email) {
        this->userId = userId;
        this->name = name;
        this->email = email;
    }

    int getId() const { return userId; }
    string getName() const { return name; }
};

// -------------------- Group --------------------

class GroupMember {
private:
    int userId;
    MemberStatus status;

public:
    GroupMember(int userId) {
        this->userId = userId;
        status = MemberStatus::ACTIVE;
    }

    int getUserId() const { return userId; }
    MemberStatus getStatus() const { return status; }

    void remove() {
        status = MemberStatus::REMOVED;
    }

    void activate() {
        status = MemberStatus::ACTIVE;
    }
};

class Group {
private:
    int groupId;
    string name;
    unordered_map<int, GroupMember> members;

public:
    Group(int groupId, string name) {
        this->groupId = groupId;
        this->name = name;
    }

    int getId() const { return groupId; }
    string getName() const { return name; }

    void addMember(int userId) {
        auto it = members.find(userId);

        if (it != members.end()) {
            it->second.activate();
            return;
        }

        members.emplace(userId, GroupMember(userId));
    }

    void removeMember(int userId) {
        auto it = members.find(userId);

        if (it != members.end()) {
            it->second.remove();
        }
    }

    bool isActiveMember(int userId) const {
        auto it = members.find(userId);

        return it != members.end() &&
               it->second.getStatus() == MemberStatus::ACTIVE;
    }

    bool hasMember(int userId) const {
        return members.find(userId) != members.end();
    }
};

// -------------------- Split Input --------------------

struct SplitInput {
    int userId;

    // Used by different strategies
    Money exactAmount = 0;
    int percentage = 0;  // Basis points: 5000 = 50%
    int shares = 0;
};

struct ExpenseSplit {
    int userId;
    Money amount;
};

// -------------------- Split Strategies --------------------

class SplitStrategy {
public:
    virtual vector<ExpenseSplit> calculate(
        Money totalAmount,
        const vector<SplitInput>& inputs
    ) = 0;

    virtual ~SplitStrategy() = default;

protected:
    void validateParticipants(
        const vector<SplitInput>& inputs
    ) {
        if (inputs.empty()) {
            throw runtime_error("No participants provided");
        }

        unordered_set<int> users;

        for (const auto& input : inputs) {
            if (!users.insert(input.userId).second) {
                throw runtime_error("Duplicate participant");
            }
        }
    }
};

// Equal Split

class EqualSplitStrategy : public SplitStrategy {
public:
    vector<ExpenseSplit> calculate(
        Money totalAmount,
        const vector<SplitInput>& inputs
    ) override {

        if (totalAmount <= 0) {
            throw runtime_error("Invalid expense amount");
        }

        validateParticipants(inputs);

        Money share = totalAmount / inputs.size();
        Money remaining = totalAmount;

        vector<ExpenseSplit> result;

        for (int i = 0; i < inputs.size(); i++) {
            Money amount;

            if (i == inputs.size() - 1) {
                amount = remaining;
            } else {
                amount = share;
            }

            result.push_back({inputs[i].userId, amount});
            remaining -= amount;
        }

        return result;
    }
};

// Exact Split

class ExactSplitStrategy : public SplitStrategy {
public:
    vector<ExpenseSplit> calculate(
        Money totalAmount,
        const vector<SplitInput>& inputs
    ) override {

        if (totalAmount <= 0) {
            throw runtime_error("Invalid expense amount");
        }

        validateParticipants(inputs);

        Money sum = 0;
        vector<ExpenseSplit> result;

        for (const auto& input : inputs) {
            if (input.exactAmount < 0) {
                throw runtime_error("Invalid exact amount");
            }

            sum += input.exactAmount;

            result.push_back({
                input.userId,
                input.exactAmount
            });
        }

        if (sum != totalAmount) {
            throw runtime_error("Exact amounts do not match total");
        }

        return result;
    }
};

// Percentage Split

class PercentageSplitStrategy : public SplitStrategy {
public:
    vector<ExpenseSplit> calculate(
        Money totalAmount,
        const vector<SplitInput>& inputs
    ) override {

        if (totalAmount <= 0) {
            throw runtime_error("Invalid expense amount");
        }

        validateParticipants(inputs);

        const int TOTAL_PERCENTAGE = 10000;
        int percentageSum = 0;

        for (const auto& input : inputs) {
            if (input.percentage < 0) {
                throw runtime_error("Invalid percentage");
            }

            percentageSum += input.percentage;
        }

        if (percentageSum != TOTAL_PERCENTAGE) {
            throw runtime_error("Percentages must total 100%");
        }

        vector<ExpenseSplit> result;
        Money remaining = totalAmount;

        for (int i = 0; i < inputs.size(); i++) {
            Money amount;

            if (i == inputs.size() - 1) {
                amount = remaining;
            } else {
                amount =
                    (totalAmount * inputs[i].percentage) /
                    TOTAL_PERCENTAGE;
            }

            result.push_back({inputs[i].userId, amount});
            remaining -= amount;
        }

        return result;
    }
};

// Shares Split

class SharesSplitStrategy : public SplitStrategy {
public:
    vector<ExpenseSplit> calculate(
        Money totalAmount,
        const vector<SplitInput>& inputs
    ) override {

        if (totalAmount <= 0) {
            throw runtime_error("Invalid expense amount");
        }

        validateParticipants(inputs);

        long long totalShares = 0;

        for (const auto& input : inputs) {
            if (input.shares <= 0) {
                throw runtime_error("Shares must be positive");
            }

            totalShares += input.shares;
        }

        vector<ExpenseSplit> result;
        Money remaining = totalAmount;

        for (int i = 0; i < inputs.size(); i++) {
            Money amount;

            if (i == inputs.size() - 1) {
                amount = remaining;
            } else {
                amount =
                    (totalAmount * inputs[i].shares) /
                    totalShares;
            }

            result.push_back({inputs[i].userId, amount});
            remaining -= amount;
        }

        return result;
    }
};

// -------------------- Expense --------------------

class Expense {
private:
    int expenseId;
    int groupId;
    int createdBy;
    int paidBy;

    string description;
    Money totalAmount;

    SplitType splitType;
    vector<ExpenseSplit> splits;

    ExpenseStatus status;

public:
    Expense(
        int expenseId,
        int groupId,
        int createdBy,
        int paidBy,
        string description,
        Money totalAmount,
        SplitType splitType,
        vector<ExpenseSplit> splits
    ) {
        this->expenseId = expenseId;
        this->groupId = groupId;
        this->createdBy = createdBy;
        this->paidBy = paidBy;
        this->description = description;
        this->totalAmount = totalAmount;
        this->splitType = splitType;
        this->splits = splits;
        status = ExpenseStatus::ACTIVE;
    }

    int getId() const { return expenseId; }
    int getGroupId() const { return groupId; }
    int getCreatedBy() const { return createdBy; }
    int getPaidBy() const { return paidBy; }

    Money getTotalAmount() const { return totalAmount; }

    SplitType getSplitType() const { return splitType; }

    const vector<ExpenseSplit>& getSplits() const {
        return splits;
    }

    ExpenseStatus getStatus() const { return status; }

    void update(
        string description,
        Money totalAmount,
        SplitType splitType,
        vector<ExpenseSplit> splits
    ) {
        this->description = description;
        this->totalAmount = totalAmount;
        this->splitType = splitType;
        this->splits = splits;
    }

    void markDeleted() {
        status = ExpenseStatus::DELETED;
    }
};

// -------------------- Ledger --------------------

class LedgerEntry {
private:
    int groupId;
    int userId;
    int sourceId;

    Money amount;
    LedgerType type;

public:
    LedgerEntry(
        int groupId,
        int userId,
        int sourceId,
        Money amount,
        LedgerType type
    ) {
        this->groupId = groupId;
        this->userId = userId;
        this->sourceId = sourceId;
        this->amount = amount;
        this->type = type;
    }

    int getGroupId() const { return groupId; }
    int getUserId() const { return userId; }
    int getSourceId() const { return sourceId; }

    Money getAmount() const { return amount; }
    LedgerType getType() const { return type; }
};

// -------------------- Settlement --------------------

class Settlement {
private:
    int settlementId;
    int groupId;
    int fromUserId;
    int toUserId;

    Money amount;

public:
    Settlement(
        int settlementId,
        int groupId,
        int fromUserId,
        int toUserId,
        Money amount
    ) {
        this->settlementId = settlementId;
        this->groupId = groupId;
        this->fromUserId = fromUserId;
        this->toUserId = toUserId;
        this->amount = amount;
    }

    int getId() const { return settlementId; }
    int getGroupId() const { return groupId; }
    int getFromUserId() const { return fromUserId; }
    int getToUserId() const { return toUserId; }
    Money getAmount() const { return amount; }
};

// -------------------- Splitwise Manager --------------------

class SplitwiseManager {
private:
    int nextUserId = 1;
    int nextGroupId = 1;
    int nextExpenseId = 1;
    int nextSettlementId = 1;

    unordered_map<int, User> users;
    unordered_map<int, Group> groups;
    unordered_map<int, Expense> expenses;

    vector<LedgerEntry> ledger;
    vector<Settlement> settlements;

    unordered_map<SplitType, unique_ptr<SplitStrategy>> strategies;

public:
    SplitwiseManager() {
        strategies[SplitType::EQUAL] =
            make_unique<EqualSplitStrategy>();

        strategies[SplitType::EXACT] =
            make_unique<ExactSplitStrategy>();

        strategies[SplitType::PERCENTAGE] =
            make_unique<PercentageSplitStrategy>();

        strategies[SplitType::SHARES] =
            make_unique<SharesSplitStrategy>();
    }

    // -------------------- User Operations --------------------

    int createUser(string name, string email) {
        int id = nextUserId++;

        users.emplace(
            piecewise_construct,
            forward_as_tuple(id),
            forward_as_tuple(id, name, email)
        );

        return id;
    }

    bool userExists(int userId) const {
        return users.find(userId) != users.end();
    }

    // -------------------- Group Operations --------------------

    int createGroup(string name) {
        int id = nextGroupId++;

        groups.emplace(
            piecewise_construct,
            forward_as_tuple(id),
            forward_as_tuple(id, name)
        );

        return id;
    }

    void addMember(int groupId, int userId) {
        if (!userExists(userId)) {
            throw runtime_error("User does not exist");
        }

        auto it = groups.find(groupId);

        if (it == groups.end()) {
            throw runtime_error("Group does not exist");
        }

        it->second.addMember(userId);
    }

    void removeMember(int groupId, int userId) {
        auto it = groups.find(groupId);

        if (it == groups.end()) {
            throw runtime_error("Group does not exist");
        }

        it->second.removeMember(userId);
    }

    bool isActiveMember(int groupId, int userId) const {
        auto it = groups.find(groupId);

        return it != groups.end() &&
               it->second.isActiveMember(userId);
    }

    // -------------------- Validation --------------------

private:
    vector<ExpenseSplit> calculateSplits(
        int groupId,
        Money totalAmount,
        SplitType splitType,
        const vector<SplitInput>& inputs
    ) {
        if (totalAmount <= 0) {
            throw runtime_error("Expense amount must be positive");
        }

        auto strategyIt = strategies.find(splitType);

        if (strategyIt == strategies.end()) {
            throw runtime_error("Invalid split type");
        }

        // Validate participants before calculating.
        for (const auto& input : inputs) {
            if (!isActiveMember(groupId, input.userId)) {
                throw runtime_error(
                    "Expense participant is not an active group member"
                );
            }
        }

        return strategyIt->second->calculate(totalAmount, inputs);
    }

    void validateExpenseUsers(
        int groupId,
        int createdBy,
        int paidBy
    ) {
        if (!userExists(createdBy) || !userExists(paidBy)) {
            throw runtime_error("Invalid user");
        }

        if (!isActiveMember(groupId, createdBy)) {
            throw runtime_error("Creator is not an active group member");
        }

        if (!isActiveMember(groupId, paidBy)) {
            throw runtime_error("Payer is not an active group member");
        }
    }

    // -------------------- Ledger Operations --------------------

    void postExpenseLedger(const Expense& expense) {
        int groupId = expense.getGroupId();
        int expenseId = expense.getId();

        // Payer receives credit for the full amount.
        ledger.emplace_back(
            groupId,
            expense.getPaidBy(),
            expenseId,
            expense.getTotalAmount(),
            LedgerType::EXPENSE
        );

        // Every participant is debited their allocated share.
        for (const auto& split : expense.getSplits()) {
            ledger.emplace_back(
                groupId,
                split.userId,
                expenseId,
                -split.amount,
                LedgerType::EXPENSE
            );
        }
    }

    void reverseExpenseLedger(int expenseId) {
        // First calculate current net impact of this expense.
        // This includes earlier reversal entries as well.
        unordered_map<int, Money> currentImpact;

        int groupId = -1;

        for (const auto& entry : ledger) {
            if (entry.getSourceId() == expenseId &&
                (entry.getType() == LedgerType::EXPENSE ||
                 entry.getType() == LedgerType::REVERSAL)) {

                currentImpact[entry.getUserId()] += entry.getAmount();
                groupId = entry.getGroupId();
            }
        }

        // Append opposite entries. Do not modify the vector
        // while iterating over it.
        for (const auto& [userId, amount] : currentImpact) {
            if (amount == 0) {
                continue;
            }

            ledger.emplace_back(
                groupId,
                userId,
                expenseId,
                -amount,
                LedgerType::REVERSAL
            );
        }
    }

    // -------------------- Expense Operations --------------------

public:
    int addExpense(
        int groupId,
        int createdBy,
        int paidBy,
        string description,
        Money totalAmount,
        SplitType splitType,
        const vector<SplitInput>& inputs
    ) {
        auto groupIt = groups.find(groupId);

        if (groupIt == groups.end()) {
            throw runtime_error("Group does not exist");
        }

        validateExpenseUsers(groupId, createdBy, paidBy);

        // Calculate and validate everything before modifying state.
        vector<ExpenseSplit> splits = calculateSplits(
            groupId, totalAmount, splitType, inputs
        );

        int expenseId = nextExpenseId++;

        Expense expense(
            expenseId,
            groupId,
            createdBy,
            paidBy,
            description,
            totalAmount,
            splitType,
            splits
        );

        expenses.emplace(expenseId, expense);

        postExpenseLedger(expenses.at(expenseId));

        return expenseId;
    }

    void editExpense(
        int expenseId,
        string description,
        Money totalAmount,
        SplitType splitType,
        const vector<SplitInput>& inputs
    ) {
        auto it = expenses.find(expenseId);

        if (it == expenses.end()) {
            throw runtime_error("Expense does not exist");
        }

        Expense& expense = it->second;

        if (expense.getStatus() == ExpenseStatus::DELETED) {
            throw runtime_error("Cannot edit deleted expense");
        }

        // Validate and calculate before changing the ledger.
        vector<ExpenseSplit> newSplits = calculateSplits(
            expense.getGroupId(),
            totalAmount,
            splitType,
            inputs
        );

        // Reverse the expense's current financial impact.
        reverseExpenseLedger(expenseId);

        // Update expense details.
        expense.update(
            description,
            totalAmount,
            splitType,
            newSplits
        );

        // Post its new financial impact.
        postExpenseLedger(expense);
    }

    void deleteExpense(int expenseId) {
        auto it = expenses.find(expenseId);

        if (it == expenses.end()) {
            throw runtime_error("Expense does not exist");
        }

        Expense& expense = it->second;

        if (expense.getStatus() == ExpenseStatus::DELETED) {
            throw runtime_error("Expense already deleted");
        }

        reverseExpenseLedger(expenseId);
        expense.markDeleted();
    }

    // -------------------- Balance Calculation --------------------

    // Positive amount means user should receive.
    // Negative amount means user owes.
    unordered_map<int, Money> getNetBalances(int groupId) const {
        unordered_map<int, Money> balances;

        for (const auto& entry : ledger) {
            if (entry.getGroupId() == groupId) {
                balances[entry.getUserId()] += entry.getAmount();
            }
        }

        return balances;
    }

    // Simplified who-owes-whom representation.
    struct Debt {
        int fromUserId;
        int toUserId;
        Money amount;
    };

    vector<Debt> getSimplifiedBalances(int groupId) const {
        auto balances = getNetBalances(groupId);

        vector<pair<int, Money>> debtors;
        vector<pair<int, Money>> creditors;

        for (const auto& [userId, amount] : balances) {
            if (amount < 0) {
                debtors.push_back({userId, -amount});
            } else if (amount > 0) {
                creditors.push_back({userId, amount});
            }
        }

        // Deterministic output.
        sort(debtors.begin(), debtors.end());
        sort(creditors.begin(), creditors.end());

        vector<Debt> result;

        int i = 0;
        int j = 0;

        while (i < debtors.size() && j < creditors.size()) {
            Money amount = min(
                debtors[i].second,
                creditors[j].second
            );

            result.push_back({
                debtors[i].first,
                creditors[j].first,
                amount
            });

            debtors[i].second -= amount;
            creditors[j].second -= amount;

            if (debtors[i].second == 0) {
                i++;
            }

            if (creditors[j].second == 0) {
                j++;
            }
        }

        return result;
    }

    // -------------------- Settlement --------------------

    void settle(
        int groupId,
        int fromUserId,
        int toUserId,
        Money amount
    ) {
        if (amount <= 0) {
            throw runtime_error("Settlement amount must be positive");
        }

        if (fromUserId == toUserId) {
            throw runtime_error("Cannot settle with yourself");
        }

        if (!groups.count(groupId)) {
            throw runtime_error("Group does not exist");
        }

        if (!userExists(fromUserId) || !userExists(toUserId)) {
            throw runtime_error("Invalid user");
        }

        // Users may have left the group but still have outstanding
        // financial obligations, so active membership is not required.

        auto balances = getNetBalances(groupId);

        Money fromBalance = balances[fromUserId];
        Money toBalance = balances[toUserId];

        // Under this netting policy, payer must owe money and
        // receiver must be owed money in this group.
        if (fromBalance >= 0 || toBalance <= 0) {
            throw runtime_error("Invalid settlement direction");
        }

        if (amount > -fromBalance || amount > toBalance) {
            throw runtime_error("Settlement exceeds outstanding balance");
        }

        int settlementId = nextSettlementId++;

        settlements.emplace_back(
            settlementId,
            groupId,
            fromUserId,
            toUserId,
            amount
        );

        // Debtor's negative balance moves towards zero.
        ledger.emplace_back(
            groupId,
            fromUserId,
            settlementId,
            amount,
            LedgerType::SETTLEMENT
        );

        // Creditor's positive balance moves towards zero.
        ledger.emplace_back(
            groupId,
            toUserId,
            settlementId,
            -amount,
            LedgerType::SETTLEMENT
        );
    }

    // -------------------- Display --------------------

    void showBalances(int groupId) const {
        cout << "\nBalances for Group " << groupId << ":\n";

        auto balances = getNetBalances(groupId);

        for (const auto& [userId, amount] : balances) {
            if (amount > 0) {
                cout << users.at(userId).getName()
                     << " should receive Rs. "
                     << amount / 100.0 << "\n";
            } else if (amount < 0) {
                cout << users.at(userId).getName()
                     << " owes Rs. "
                     << (-amount) / 100.0 << "\n";
            }
        }
    }

    void showSimplifiedBalances(int groupId) const {
        cout << "\nSimplified balances for Group "
             << groupId << ":\n";

        auto debts = getSimplifiedBalances(groupId);

        if (debts.empty()) {
            cout << "No outstanding balances\n";
            return;
        }

        for (const auto& debt : debts) {
            cout << users.at(debt.fromUserId).getName()
                 << " owes "
                 << users.at(debt.toUserId).getName()
                 << " Rs. "
                 << debt.amount / 100.0 << "\n";
        }
    }
};

// -------------------- Main --------------------

int main() {
    SplitwiseManager manager;

    // Create users
    int A = manager.createUser("A", "a@gmail.com");
    int B = manager.createUser("B", "b@gmail.com");
    int C = manager.createUser("C", "c@gmail.com");

    // Create two groups
    int G1 = manager.createGroup("Trip");
    int G2 = manager.createGroup("Flat");

    // Add same users to both groups
    for (int userId : {A, B, C}) {
        manager.addMember(G1, userId);
        manager.addMember(G2, userId);
    }

    // ------------------------------------------------
    // G1: A pays Rs. 1200, equally split among A, B, C
    // ------------------------------------------------

    vector<SplitInput> equalSplit = {
        {A},
        {B},
        {C}
    };

    int expense1 = manager.addExpense(
        G1,
        A,
        A,
        "Dinner",
        120000,
        SplitType::EQUAL,
        equalSplit
    );

    manager.showBalances(G1);
    manager.showSimplifiedBalances(G1);

    // ------------------------------------------------
    // G2: B pays Rs. 1000, split using shares 2:1:1
    // ------------------------------------------------

    vector<SplitInput> shareSplit = {
        {A, 0, 0, 2},
        {B, 0, 0, 1},
        {C, 0, 0, 1}
    };

    manager.addExpense(
        G2,
        B,
        B,
        "Groceries",
        100000,
        SplitType::SHARES,
        shareSplit
    );

    manager.showBalances(G2);
    manager.showSimplifiedBalances(G2);

    // ------------------------------------------------
    // Settle Rs. 300 from B to A in G1
    // ------------------------------------------------

    manager.settle(G1, B, A, 30000);

    cout << "\nAfter settlement in G1:\n";
    manager.showBalances(G1);

    // G2 remains unaffected.
    cout << "\nG2 remains unchanged:\n";
    manager.showBalances(G2);

    // ------------------------------------------------
    // Edit the first expense
    // ------------------------------------------------

    vector<SplitInput> exactSplit = {
        {A, 40000},
        {B, 40000},
        {C, 40000}
    };

    manager.editExpense(
        expense1,
        "Updated Dinner",
        120000,
        SplitType::EXACT,
        exactSplit
    );

    cout << "\nAfter editing dinner in G1:\n";
    manager.showBalances(G1);

    // ------------------------------------------------
    // Delete the first expense
    // ------------------------------------------------

    manager.deleteExpense(expense1);

    cout << "\nAfter deleting dinner in G1:\n";
    manager.showBalances(G1);

    return 0;
}
