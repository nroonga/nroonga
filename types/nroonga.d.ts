declare namespace nroonga {
  interface CallbackType {
    (error: Error, data: any) => void
  }

  class Database {
    constructor(path?: string, openOnly?: boolean)

    close(): void

    command(command: string, callback: CallbackType): void
    command(command: string, options: object, callback: CallbackType): void

    commandSync(command: string, options?: object): any
  }
}

export = nroonga
