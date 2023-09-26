import asyncio
import aiohttp
import datetime as dt

async def get(url:str, session:aiohttp.ClientSession, **kwargs):
    async with session.get(url, **kwargs) as response:
        json = await response.json()
        return json

async def main():
    url = "http://localhost:8080/stock_data/PETR4?start_date=2023-09-01&end_date=2023-09-26"
    async with aiohttp.ClientSession() as session:
        start = dt.datetime.now()
        async with asyncio.TaskGroup() as group:
            for _ in range(10000):
                group.create_task(get(url, session))
        end = dt.datetime.now()
        print(f"Time: {end - start}")

if __name__ == "__main__":
    asyncio.run(main())