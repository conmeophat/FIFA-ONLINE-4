# Football data policy

## Fixture data

Everything currently under `data/fixtures/` is synthetic test content. Names such as "Demo Blue FC" and "Sample Striker" are fictional and exist only to exercise the API. They must never be shown as real-world football records.

## Production data admission checklist

Before importing any real-world club, competition, player, attribute, crest, kit, photograph, stadium or likeness, record:

1. Provider and exact source URL or dataset identifier.
2. License / contract / permission and redistribution restrictions.
3. Attribution requirements.
4. Retrieval timestamp and dataset version.
5. Entity identifier strategy and deduplication rules.
6. Verification state and confidence for each field.
7. Update cadence, correction process and historical retention rules.

A public webpage is not automatically a license to scrape, copy, redistribute or commercially use its database or images. Use an API or dataset only within its terms. Do not bundle protected logos, photographs, kits, player likenesses or proprietary game assets without permission.

## Required record provenance

Every imported production record should retain source metadata. Do not silently replace unknown values with guesses. Use null/unknown values where appropriate, and distinguish current club registration from historical career records.

## Player ratings

Do not present project-generated ratings as official ratings. If the project calculates gameplay attributes, label them as in-game estimates and document the formula/version separately from real-world performance facts.
