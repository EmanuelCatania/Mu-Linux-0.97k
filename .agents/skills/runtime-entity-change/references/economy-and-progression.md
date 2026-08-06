# Economy and progression checklist

Use when an entity is obtained or consumed through shops, drops, rewards,
events, quests, or mixes.

## Check

- Identify the authoritative table or code path for eligibility, probability,
  price, currency, quantity, ingredients, taxes, and result generation.
- Keep decisions and mutations on the server; validate inventory capacity and
  rollback before consuming currency, ingredients, or claims.
- Verify item encoding, generated options, durability, expiration, binding,
  ownership, logs, persistence, and reload/restart semantics.
- Cover duplicate claims, repeated or concurrent requests, disconnects, and
  failure paths without double consumption or reward.
- Review web/editor validation only when operators can modify the authoritative
  source.
- Keep structural support, balance values, and live availability changes
  separately reviewable when practical.

## Validate

Test zero/minimum/normal/maximum values, invalid references, insufficient
resources, full inventory, success and failure, concurrent/repeated requests,
reconnect/restart, logs, persistence, and one neighboring legacy entry.
