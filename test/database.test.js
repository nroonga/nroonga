/* eslint no-unused-expressions: 0 */
'use strict'

const fs = require('fs')
const path = require('path')
const expect = require('chai').expect
const nroonga = require('../lib/nroonga')

const testData = [{
  _key: 'http://groonga.org/',
  title: 'groonga - An open-source fulltext search engine and column store'
}, {
  _key: 'http://groonga.rubyforge.org/',
  title: 'Fulltext search by Ruby with groonga - Ranguba'
}, {
  _key: 'http://mroonga.github.com/',
  title: 'Groonga storage engine - Fast fulltext search on MySQL'
}]

const temporaryDatabase = callback => {
  const tempdir = path.join('test', 'tmp')
  fs.mkdir(tempdir, () => {
    const databaseName = `tempdb-${process.pid}-${(new Date()).valueOf()}`
    const db = new nroonga.Database(path.join(tempdir, databaseName))

    try {
      callback(db)
    } finally {
      db.close()
      fs.readdir(tempdir, (err, files) => {
        if (err) throw err
        const re = RegExp('^' + databaseName)
        files.forEach(file => {
          if (file.match(re)) fs.unlinkSync(path.join(tempdir, file))
        })
      })
    }
  })
}

const withTestDatabase = callback => {
  temporaryDatabase(db => {
    db.commandSync('table_create', {
      name: 'Site',
      flags: 'TABLE_HASH_KEY',
      key_type: 'ShortText'
    })
    db.commandSync('column_create', {
      table: 'Site',
      name: 'title',
      flags: 'COLUMN_SCALAR',
      type: 'ShortText'
    })

    db.commandSync('table_create', {
      name: 'Terms',
      flags: 'TABLE_PAT_KEY|KEY_NORMALIZE',
      key_type: 'ShortText',
      default_tokenizer: 'TokenBigram'
    })
    db.commandSync('column_create', {
      table: 'Terms',
      name: 'entry_title',
      flags: 'COLUMN_INDEX|WITH_POSITION',
      type: 'Site',
      source: 'title'
    })

    db.commandSync('load', {
      table: 'Site',
      values: JSON.stringify(testData)
    })

    callback(db)
  })
}

/* global beforeEach, describe, it */
describe('nroonga.Database', () => {
  let db = null
  beforeEach(() => {
    db = new nroonga.Database()
  })

  describe('#commandSync', () => {
    it('should return groonga results', () => {
      const status = db.commandSync('status')
      expect(status).to.have.property('version')
    })
  })

  describe('#command', () => {
    it('should return groonga results', done => {
      db.command('status', (error, data) => {
        expect(error).to.be.null
        expect(data).to.have.property('version')
        done()
      })
    })
  })

  describe('duplicated #close call', () => {
    it('should raise an exception', () => {
      db.close()
      expect(() => db.close()).to.throw('Database already closed')
    })
  })

  describe('#commandSync for closed database', () => {
    it('should raise an exception', () => {
      db.close()
      expect(() => db.commandSync('status')).to.throw('Database already closed')
    })
  })

  describe('#command for closed database', () => {
    it('should return an error', () => {
      db.close()
      expect(() => {
        db.command('status', () => {
          // do nothing
        })
      }).to.throw('Database already closed')
    })
  })
})

describe('empty database', () => {
  const db = new nroonga.Database()

  describe('#dump', () => {
    it('should return empty string', () => {
      const result = db.commandSync('dump')
      expect(result).to.be.empty
    })
  })
})

describe('database whose name is not string', () => {
  it('should throw an exception', () => {
    expect(() => new nroonga.Database(1)).to.throw()
  })
})

describe('database with data stored', () => {
  it('should select records', done => {
    withTestDatabase(db => {
      const matched = db.commandSync('select', {table: 'Site'})
      expect(matched[0][0][0]).to.equal(3)
      done()
    })
  })

  it('should select records ignoring the null valued option', done => {
    withTestDatabase(db => {
      const matched = db.commandSync('select', {
        table: 'Site',
        query: null
      })
      expect(matched[0][0][0]).to.equal(3)
      done()
    })
  })

  it('should search by query', done => {
    withTestDatabase(db => {
      const matched = db.commandSync('select', {
        table: 'Site',
        match_columns: 'title',
        query: 'ruby'
      })
      expect(matched[0][0][0]).to.equal(1)
      done()
    })
  })

  it('should search by query including space', done => {
    withTestDatabase(db => {
      const matched = db.commandSync('select', {
        table: 'Site',
        match_columns: 'title',
        query: 'search ranguba'
      })
      expect(matched[0][0][0]).to.equal(1)
      done()
    })
  })

  it('should dump all records', done => {
    withTestDatabase(db => {
      const expectedDump = `table_create Site TABLE_HASH_KEY ShortText
column_create Site title COLUMN_SCALAR ShortText

table_create Terms TABLE_PAT_KEY ShortText \
--default_tokenizer TokenBigram --normalizer NormalizerAuto

load --table Site
[
["_key","title"],
["http://groonga.org/",\
"groonga - An open-source fulltext search engine and column store"],
["http://groonga.rubyforge.org/",\
"Fulltext search by Ruby with groonga - Ranguba"],
["http://mroonga.github.com/",\
"Groonga storage engine - Fast fulltext search on MySQL"]
]

column_create Terms entry_title COLUMN_INDEX|WITH_POSITION Site title`

      const result = db.commandSync('dump', {tables: 'Site'})
      expect(result).to.equal(expectedDump)
      done()
    })
  })
})
