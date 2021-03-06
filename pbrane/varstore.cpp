#include "varstore.hpp"
using std::string;
using std::to_string;
using zidcu::Database;

#include <algorithm>
using std::transform;
using std::set_difference;

#include <iterator>
using std::inserter;

#include "sekisa/err.hpp"

#include <iostream>
using std::cerr;
using std::endl;

Variable makeVariable(string str);
Variable makeVariable(string str) { return Variable::parse(str); }

SqlVarStore::SqlVarStore(Database &db, string varTableName)
		: _db{db}, _varTable(varTableName) { }

Variable SqlVarStore::get(string name) {
	createTables();
	auto row = _db.execute(
		"SELECT body, type FROM " + _varTable + " WHERE name = ?1",
		name);
	if(row.status() == SQLITE_ROW) {
		Variable var{};
		var.type = typeFromString(row.getString(1));
		var.value = row.getString(0);
		return var;
	}

	return Variable{};
}
Variable SqlVarStore::set(string name, Variable var) {
	createTables();
	auto tran = _db.transaction();
	_db.executeVoid("INSERT OR IGNORE INTO " + _varTable + " VALUES(?1, ?2, ?3)",
			name, var.value, typeToString(var.type));
	_db.executeVoid("UPDATE " + _varTable + " SET body = ?1, type = ?2 WHERE name = ?3",
			var.value, typeToString(var.type), name);
	return var;
}
bool SqlVarStore::defined(string name) {
	createTables();
	auto result = _db.executeScalar<int>(
			"SELECT COUNT(1) FROM " + _varTable + " WHERE name = ?1",
			name);
	if(!result) throw make_except("expected count value");
	return *result;
}
void SqlVarStore::erase(string name) {
	createTables();
	_db.executeVoid("DELETE FROM " + _varTable + " WHERE name = ?1",
			name);
}

void SqlVarStore::createTables() {
	if(_tablesCreated) return;

	auto tran = _db.transaction();
	_db.executeVoid("CREATE TABLE IF NOT EXISTS " + _varTable
		+ " (name text primary key, body text, type text)");
	_db.executeVoid("CREATE INDEX IF NOT EXISTS " + _varTable + "_type_idx"
		+ " ON " + _varTable + " (type)");
	_tablesCreated = true;
}

std::set<string> SqlVarStore::get() {
	createTables();
	std::set<string> vars;
	auto results = _db.execute("SELECT name, type FROM " + _varTable);
	while(results.status() == SQLITE_ROW) {
		vars.insert(results.getString(0));
		results.step();
	}
	if(results.status() == SQLITE_DONE)
		return vars;

	throw make_except("sqlite error: " + to_string(results.status()));

}
std::set<string> SqlVarStore::getVariablesOfType(Type type) {
	createTables();
	std::set<string> vars;
	auto results = _db.execute("SELECT name, type FROM " + _varTable + " WHERE type = ?1",
			typeToString(type));
	// TODO: I'm baffled by this... I've been out of my mind for days maybe that's it
	// TODO: investigate and see if a previous query is not being closed?
	// TODO: do get queries only work because they request one element?
	// TODO: is type a kewyord?
	while(results.status() == SQLITE_ROW) {
		if(results.getString(1) != typeToString(type)) {
			cerr << "added '" << results.getString(0) << "' : "
				<< results.getString(1) << " != " << typeToString(type) << endl;
		} else {
			vars.insert(results.getString(0));
		}
		results.step();
	}
	if(results.status() == SQLITE_DONE)
		return vars;

	throw make_except("sqlite error: " + to_string(results.status()));
}

LocalVarStore::LocalVarStore() { }

Variable LocalVarStore::get(string name) {
	if(this->defined(name))
		return _vars[name];
	return Variable{};
}
Variable LocalVarStore::set(string name, Variable var) {
	return _vars[name] = var;
}

bool LocalVarStore::defined(string name) {
	return _vars.find(name) != _vars.end();
}
void LocalVarStore::erase(string name) {
	if(!this->defined(name))
		return;
	_vars.erase(_vars.find(name));
}

std::set<string> LocalVarStore::get() {
	std::set<string> vars;
	for(auto &var : _vars)
		vars.insert(var.first);
	return vars;
}
std::set<string> LocalVarStore::getVariablesOfType(Type type) {
	// TODO: this really isn't very efficient... change to
	// TODO: map<type, map<string, var>>? Way it's used isn't so bad
	std::set<string> vars;
	for(auto &var : _vars)
		if(var.second.type == type)
			vars.insert(var.first);
	return vars;
}


// TODO: caching?
TransactionalVarStore::TransactionalVarStore(VarStore &store) : _store{store} { }
TransactionalVarStore::~TransactionalVarStore() {
	if(!_commit)
		return;

	auto lvars = _lstore.get();
	for(auto &var : lvars)
		_store.set(var, _lstore.get(var));
	for(auto &var : _erased)
		_store.erase(var);
}

Variable TransactionalVarStore::get(string name) {
	if(_erased.count(name) > 0)
		return Variable{};
	if(_lstore.defined(name))
		return _lstore.get(name);
	if(_store.defined(name))
		return _store.get(name);
	return Variable{};
}
Variable TransactionalVarStore::set(string name, Variable var) {
	_erased.erase(name);
	return _lstore.set(name, var);
}

bool TransactionalVarStore::defined(string name) {
	if(_erased.count(name) > 0)
		return false;
	if(_lstore.defined(name))
		return true;
	return _store.defined(name);
}
void TransactionalVarStore::erase(string name) {
	_erased.insert(name);
	if(_lstore.defined(name))
		_lstore.erase(name);
}

std::set<string> TransactionalVarStore::get() {
	auto svars = _store.get();
	auto lvars = _lstore.get();

	return this->filterErased(svars, lvars);
}

std::set<string> TransactionalVarStore::getVariablesOfType(Type type) {
	auto svars = _store.getVariablesOfType(type);
	auto lvars = _lstore.getVariablesOfType(type);

	return this->filterErased(svars, lvars);
}

void TransactionalVarStore::abort() { _commit = false; }

std::set<string> TransactionalVarStore::filterErased(
		std::set<string> &a, std::set<string> &b) {
	// merge together backing and local vars
	std::set<string> combined{};
	set_union(a.begin(), a.end(), b.begin(), b.end(),
			inserter(combined, combined.begin()));

	// strip out erased variables
	std::set<string> vars{};
	set_difference(combined.begin(), combined.end(),
			_erased.begin(), _erased.end(),
			inserter(vars, vars.begin()));

	return vars;
}

